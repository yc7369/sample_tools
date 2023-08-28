/**
 * @file event_client.cpp
 * @author yangchen
 * @brief event client 封装
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "event_client.h"
#include "event2/event.h"
#include "event2/bufferevent_ssl.h"
#include "util.h"
#include "event2/listener.h"
#include "zlog/ztp_log.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#ifndef FEATURE_IOMN
#pragma message("FEATURE_IOMN close")
#else
#include "iomn/metric.h"
#endif

namespace tcp {

using namespace util;

struct EventClientDataImpl {
    bufferevent* client_ = nullptr;
    bool is_connected_ = false;
    bool is_started_ = false;
    std::shared_ptr<std::thread> th_;
    SSL* ssl_ = nullptr;
    //最后一次从etcd同步而来的host，如果和现在一致,则跳过，即热备情况
    std::pair<std::string, std::string> etcd_last_hosts_;
    std::string connection_info_;
};
#define impl ((EventClientDataImpl*)pimpl_)

EventClient::EventClient(bool use_thread) : EventWrapper(use_thread) {
    pimpl_ = new EventClientDataImpl();
    impl->is_connected_ = false;
    impl->is_started_ = false;
}

EventClient::~EventClient() {
    Stop();
    EventWrapper::Stop();
    if (impl && impl->th_ && impl->th_->joinable()) {
        impl->th_->join();

        // 销毁实例
        delete impl;
        pimpl_ = nullptr;
    }
}

int EventClient::Start(const ConnectionParams& params) {
    if (impl->is_started_) {
        assert(false);
        return -1;
    }
    impl->is_started_ = true;
    setParams(params);
    impl->th_ = std::make_shared<std::thread>([this]() {
        Connect();
        Run();
    });
    return 0;
}

bool EventClient::Send(const void* data, size_t size) {
    if (!impl->client_) {
        return false;
    }
    return (0 == bufferevent_write(impl->client_, data, size));
}

int EventClient::Close() {
    if (impl->client_) {
        bufferevent_free(impl->client_);
        impl->client_ = nullptr;
    }
    impl->is_connected_ = false;
    return 0;
}

void EventClient::OnDisconnected() {
    if (impl->is_started_) {
        ReConnect();
    }
}

bool EventClient::isConnected() {
    return impl->is_connected_;
}

void EventClient::OnConnected() {
    impl->is_connected_ = true;
}

void EventClient::OnTimeout() {
    if (!impl->is_connected_) {
        Connect();
    }
}

void EventClient::TimeoutCallback(evutil_socket_t fd, short what, void* user_data) {
    if (what != EV_TIMEOUT || !user_data) {
        return;
    }
    ((EventClient*)user_data)->OnTimeout();
}

int EventClient::ReConnect() {
    timeval tv = { 0 };
    tv.tv_sec = 3000 / 1000;
    tv.tv_usec = (3000 % 1000) * 1000;

    event* ev = event_new(base(), -1, 0, EventClient::TimeoutCallback, this);
    if (!ev) {
        return -1;
    }
    event_add(ev, &tv);
    return 0;
}

void EventClient::OnEvent(bufferevent* bev, short events) {
    if (events & BEV_EVENT_EOF || events & BEV_EVENT_ERROR || events & BEV_EVENT_READING ||
        events & BEV_EVENT_WRITING) {
        impl->is_connected_ = false;
#ifdef FEATURE_IOMN
        METRIC_STATUS(impl->connection_info_ + "_connected", "false");
#endif

        ZLOG_INFO("Connect Disconnect");
        OnDisconnected();
    } else if (events & BEV_EVENT_CONNECTED) {
        impl->is_connected_ = true;
#ifdef FEATURE_IOMN
        METRIC_STATUS(impl->connection_info_ + "_connected", "true");
#endif
        ZLOG_INFO("Connect Connected");
        OnConnected();
    }
}

void* EventClient::CreateSSL() {
    if (impl->ssl_) {
        return impl->ssl_;
    }
    SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
    SSL* ssl = nullptr;
    if (ctx == NULL) {
        printf("SSL_CTX_new error!\n");
        ERR_print_errors_fp(stderr);
        ZLOG_ERROR("TcpSSLServer :CreateSSL SSL_CTX_new  error {}, {}", errno, strerror(errno));
        return NULL;
    }

    // SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    // SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);

    if (params().cert_verify) {
        std::string path = params().cert_path + "ca.crt";
        if (!SSL_CTX_load_verify_locations(ctx, path.c_str(), NULL)) {
            ZLOG_ERROR("TcpSSLClient::CreateSSL SSL_CTX_load_verify_locations error");
            ERR_print_errors_fp(stderr);
            return nullptr;
        }

        path = params().cert_path + "client.crt";
        if (SSL_CTX_use_certificate_file(ctx, path.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ZLOG_ERROR("TcpSSLClient::CreateSSL SSL_CTX_use_certificate_file error");
            ERR_print_errors_fp(stderr);
            return nullptr;
        }

        path = params().cert_path + "client.key";
        if (SSL_CTX_use_PrivateKey_file(ctx, path.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ZLOG_ERROR("TcpSSLClient::CreateSSL SSL_CTX_use_PrivateKey_file error");
            ERR_print_errors_fp(stderr);
            return nullptr;
        }

        if (!SSL_CTX_check_private_key(ctx)) {
            ZLOG_ERROR("TcpSSLClient::CreateSSL SSL_CTX_check_private_key error");
            ERR_print_errors_fp(stderr);
            return nullptr;
        }
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        ERR_print_errors_fp(stderr);
        ZLOG_ERROR("TcpSSLClient::CreateSSL SSL_new error");
        return nullptr;
    }
    return ssl;
}

int EventClient::Connect() {
    Close();

    // TODO 处理热备失联情况
    auto _connect = [&](const std::string& ip, const std::string& port) {
        ZLOG_DEBUG("tcp client connect to: {}:{}", ip, port);
        struct addrinfo* result;
        evutil_addrinfo hints = { 0 };
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICHOST;

        int ret = evutil_getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
        if ((ret != 0) || result == nullptr) {
            ZLOG_ERROR("getaddrinfo for {}:{} error: {}", ip, port, ret);
            return ret;
        }

        if (params().is_ssl && impl->ssl_ == nullptr) {
            impl->ssl_ = (SSL*)CreateSSL();
            if (!impl->ssl_) {
                ZLOG_ERROR("TcpSSLClient::CreateSSL error");
                ERR_print_errors_fp(stderr);
                return -1;
            }
        }

        int socket_flag = BEV_OPT_CLOSE_ON_FREE;

        if (use_thread()) {
            socket_flag |= BEV_OPT_THREADSAFE;
        }

        if (impl->ssl_ == nullptr) {
            impl->client_ = bufferevent_socket_new(base(), -1, socket_flag);
            ZLOG_INFO("connect with {}：{}", ip, port);
        } else {
            ZLOG_INFO("connect with ssl {}：{}", ip, port);
            impl->client_ = bufferevent_openssl_socket_new(
                base(), -1, impl->ssl_, BUFFEREVENT_SSL_CONNECTING, socket_flag);
        }
        if (impl->client_ == nullptr) {
            ZLOG_ERROR("bufferevent_socket_new return nullptr.");
            return -1;
        }
        SetTcpNoDelay(impl->client_);
        SetKeepAlive(impl->client_);
        bufferevent_setcb(impl->client_, ReadCallback, WriteCallback, EventCallback, this);
        bufferevent_enable(impl->client_, EV_READ | EV_WRITE);

        ZLOG_INFO("get address {}...", (result[0].ai_family != AF_INET6) ? "ipv4" : "ipv6");
        if (bufferevent_socket_connect(impl->client_, result[0].ai_addr, result[0].ai_addrlen) <
            0) {
            ZLOG_ERROR("connect failed: {}:{}", ip, port);
            bufferevent_free(impl->client_);
            impl->client_ = nullptr;
            return -1;
        }
        return 0;
    };
    //只要填了endpoint，默认启用etcd
    if (params().etcd_endpoints.size() && params().hostkey.size()) {

#ifdef FEATURE_IOMN
        METRIC_STATUS("etcd_use_" + params().hostkey, "true");
#endif

        std::string full_endpoints;
        if (get_full_endpoints_etcd(params().etcd_endpoints, full_endpoints)) {
            ZLOG_INFO("get_full_endpoints_etcd :{}", full_endpoints);
#ifdef FEATURE_IOMN
            METRIC_STATUS("etcd_full_endpoints", full_endpoints);
#endif
            params().etcd_endpoints = full_endpoints;
        } else {
            ZLOG_ERROR("get_full_endpoints_etcd fail");
        }

        std::vector<std::pair<std::string, std::string>> host;
        if (get_master_host_from_etcd(params().etcd_endpoints, params().hostkey, host)) {
            ZLOG_INFO("tcp client get_master_host: {} success,host size:{}",
                      params().etcd_endpoints,
                      host.size());

#ifdef FEATURE_IOMN
            METRIC_STATUS("etcd_connected", "true");
#endif

            for (auto p : host) {
                if (host.size() > 1 && impl->etcd_last_hosts_ == p) {
                    continue;
                }
#ifdef FEATURE_IOMN
                METRIC_COUNT(std::string("etcd_") + params().hostkey + "_" + p.first + ":" +
                             p.second);
#endif

                if (_connect(p.first, p.second) == 0) {
                    ZLOG_INFO("tcp client connect with etcd: {}:{}", p.first, p.second);
                    impl->etcd_last_hosts_ = p;
                    impl->connection_info_ = "etcd_" + p.first + ":" + p.second;

                    return 0;
                } else {
                    ZLOG_WARN("tcp client connect with etcd: {}:{} fail", p.first, p.second);
                }
            }
        } else {
            ZLOG_ERROR("get_master_host fail {}", params().etcd_endpoints);
#ifdef FEATURE_IOMN
            METRIC_STATUS("etcd_connected", "false");
#endif
        }
    }

    //如果连接etcd失败，使用默认url进行连接
    impl->etcd_last_hosts_ = {};
    impl->connection_info_ = "default_" + params().address + ":" + params().port;
#ifdef FEATURE_IOMN
    METRIC_COUNT(params().address + ":" + params().port);
#endif

    return _connect(params().address, params().port);
}

int EventClient::Stop() {
    impl->is_started_ = false;
    Close();
    return 0;
}

}  // namespace tcp
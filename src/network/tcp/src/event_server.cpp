/**
 * @file event_server.cpp
 * @author yangchen
 * @brief event server 封装
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "event_server.h"
#include "util.h"
#include "event2/listener.h"
#include "event2/event.h"
#include "event2/bufferevent_ssl.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#ifdef FEATURE_IOMN
#include "iomn/metric.h"
#endif

namespace tcp {

using namespace util;

struct EventServerDataImpl {
    evconnlistener* listener_ = nullptr;
};
#define impl ((EventServerDataImpl*)pimpl_)

EventServer::EventServer(bool use_thread) : EventWrapper(use_thread) {
    pimpl_ = new EventServerDataImpl();
}

EventServer::~EventServer() {
    if (impl && impl->listener_) {
        evconnlistener_free(impl->listener_);

        delete impl;
        pimpl_ = nullptr;
    }
}

typedef void (*evconnlistener_cb)(
    struct evconnlistener*, evutil_socket_t, struct sockaddr*, int socklen, void*);

void ListenerCallback(struct evconnlistener* listener,
                      evutil_socket_t fd,
                      struct sockaddr* sa,
                      int socklen,
                      void* user_data) {
    auto p = (EventServer*)user_data;
    struct event_base* base = p->base();

    bufferevent* bev = nullptr;

    int socket_flag = BEV_OPT_CLOSE_ON_FREE;

    if (p->use_thread()) {
        socket_flag |= BEV_OPT_THREADSAFE;
    }

    if (p->params().is_ssl) {
        SSL* ssl = (SSL*)(p->CreateSSL());
        bev = bufferevent_openssl_socket_new(base, fd, ssl, BUFFEREVENT_SSL_ACCEPTING, socket_flag);
    } else {
        bev = bufferevent_socket_new(base, fd, socket_flag);  //|BEV_OPT_THREADSAFE
    }

    if (!bev) {
        ZLOG_ERROR("TcpServer::ListenerCallback socket new failed");
        return;
    }

    EventWrapper::SetTcpNoDelay(bev);
    p->OnAccept(bev, fd, sa, socklen);
    bufferevent_setcb(bev,
                      EventWrapper::ReadCallback,
                      EventWrapper::WriteCallback,
                      EventWrapper::EventCallback,
                      user_data);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void* EventServer::CreateSSL() {

    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    ctx = SSL_CTX_new(SSLv23_method());
    if (ctx == NULL) {
        printf("SSL_CTX_new error!\n");
        ERR_print_errors_fp(stderr);
        ZLOG_ERROR("TcpSSLServer :CreateSSL SSL_CTX_new  error {}, {}", errno, strerror(errno));
        return NULL;
    }

    SSL_CTX_set_verify(ctx,
                       SSL_VERIFY_PEER | (params().cert_verify ? SSL_VERIFY_FAIL_IF_NO_PEER_CERT
                                                               : SSL_VERIFY_NONE),
                       NULL);

    std::string path = params().cert_path + "ca.crt";
    if (!SSL_CTX_load_verify_locations(ctx, path.c_str(), NULL)) {
        ZLOG_ERROR("TcpSSLServer::CreateSSL SSL_CTX_load_verify_locations error");
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    path = params().cert_path + "server.crt";
    if (SSL_CTX_use_certificate_file(ctx, path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ZLOG_ERROR("TcpSSLServer::CreateSSL SSL_CTX_use_certificate_file error");
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    path = params().cert_path + "server.key";
    if (SSL_CTX_use_PrivateKey_file(ctx, path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ZLOG_ERROR("TcpSSLServer::CreateSSL SSL_CTX_use_PrivateKey_file error");
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        ZLOG_ERROR("TcpSSLServer::CreateSSL SSL_CTX_check_private_key error");
        ERR_print_errors_fp(stderr);
        return nullptr;
    }
    // SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    ssl = SSL_new(ctx);
    if (!ssl) {
        ZLOG_ERROR("TcpSSLServer::CreateSSL SSL_new error");
        return nullptr;
    }
    ZLOG_INFO("create ssl success");
    return ssl;
}

int EventServer::Start(const ConnectionParams& conn_params) {

    if (conn_params.address.size() == 0) {
        ZLOG_WARN("Attention!! No Listen Address!!!");
        return 0;
    }

    setParams(conn_params);

    if (impl->listener_) {
        ZLOG_ERROR("TcpServer::Start error: listener is NOT null, already started?");
        return -1;
    }

    if (params().etcd_endpoints.size() && params().hostkey.size()) {
        std::string full_endpoints;

#ifdef FEATURE_IOMN
        METRIC_STATUS("etcd_server_key", params().etcd_endpoints + "_" + params().hostkey);
#endif

        if (get_full_endpoints_etcd(params().etcd_endpoints, full_endpoints)) {
            ZLOG_INFO("get_full_endpoints_etcd :{}", full_endpoints);
#ifdef FEATURE_IOMN
            METRIC_STATUS("etcd_full_endpoints", full_endpoints);
#endif

            params().etcd_endpoints = full_endpoints;
        } else {
            ZLOG_ERROR("get_full_endpoints_etcd fail");
        }

        std::string hostvalue = params().etcd_address + ":" + params().etcd_port;
        if (set_key_to_etcd(params().etcd_endpoints, params().hostkey, hostvalue)) {
            ZLOG_INFO("set_key_to_etcd :{}", hostvalue);
        } else {
            ZLOG_ERROR("set_key_to_etcd fail");
        }
    }

    struct addrinfo* result = nullptr;
    evutil_addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;

    int ret = evutil_getaddrinfo(params().address.c_str(), params().port.c_str(), &hints, &result);
    if (ret != 0 || result == nullptr) {
        ZLOG_ERROR(
            "TcpServer:resolve address error: {}:{} {}", params().address, params().port, ret);
        return -1;
    }
#ifdef FEATURE_IOMN
    METRIC_STATUS("listen",
                  ((result[0].ai_family != AF_INET6) ? std::string("ipv4") : "ipv6") + "_" +
                      params().address + "_" + params().port);
#endif
    ZLOG_INFO("get address {}...", (result[0].ai_family != AF_INET6) ? "ipv4" : "ipv6");

    impl->listener_ = evconnlistener_new_bind(base(),
                                              ListenerCallback,
                                              this,
                                              LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                              10,
                                              result[0].ai_addr,
                                              result[0].ai_addrlen);
    if (!impl->listener_) {
        ZLOG_CRITICAL("TcpServer:start error, listener is null errno:{}!", errno);
        exit(-1);
        return -1;
    }
    ZLOG_INFO("listen at {}:{}success ", params().address, params().port);
    return 0;
}

int EventServer::Close(bufferevent* bev) {
    bufferevent_disable(bev, EV_READ | EV_WRITE);
    if (params().is_ssl) {
        SSL* ssl = bufferevent_openssl_get_ssl(bev);
        if (ssl) {
            SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
            SSL_shutdown(ssl);
            ssl = nullptr;
        }
    }

    bufferevent_free(bev);
    return 0;
}
}  // namespace tcp
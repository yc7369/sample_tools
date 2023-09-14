#include "linker_impl_server.h"

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#endif  // _MSC_VER

#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include "msg_functions.h"
#include "event2/buffer.h"
#include "util.h"
#include "mybase/timerman.h"
#include "iomn/metric.h"

namespace linker {

XTcpServer::XTcpServer() : tcp::EventServer(false) {}

void XTcpServer::start_run_thread() {
    run_thread_ = std::make_shared<std::thread>([this]() { this->Run(); });
    run_thread_->detach();
}

bool XTcpServer::init(const std::string& host) {
    config_ = host;
#ifndef _MSC_VER
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == 0) {
        ZLOG_INFO("SIGPIPE ignore\n");
    } else {
        ZLOG_ERROR("SIGPIPE ignore\n");
    }
#endif
    return true;
}

XTcpServer::~XTcpServer() {}

int64_t _connect_count = 0;
void XTcpServer::OnAccept(bufferevent* bev, evutil_socket_t fd, struct sockaddr* sa, int socklen) {
    int port = ntohs(((sockaddr_in*)sa)->sin_port);
    char ip[128] = { 0 };
    if (sa->sa_family == AF_INET) {
        inet_ntop(sa->sa_family, &((sockaddr_in*)sa)->sin_addr, ip, sizeof(ip));
    } else {
        inet_ntop(sa->sa_family, &((sockaddr_in6*)sa)->sin6_addr, ip, sizeof(ip));
    }

    ZLOG_INFO("Accept {} ip:{} port:{}", (int64_t)bev, ip, port);
    METRIC_GUAGE("connect_count", ++_connect_count);
}

void XTcpServer::OnEvent(bufferevent* bev, short events) {
    if (events & BEV_EVENT_EOF || events & BEV_EVENT_ERROR || events & BEV_EVENT_READING ||
        events & BEV_EVENT_WRITING) {
        Close(bev);
        METRIC_GUAGE("connect_count", --_connect_count);

        ZLOG_INFO("close {}", (int64_t)bev);
    } else {
        // std::cout << "OnEvent " << bev << " evs:" << events << std::endl;
    }
}

void XTcpServer::OnRead(bufferevent* bev) {
    struct evbuffer* input = bufferevent_get_input(bev);
    while (true) {
        const size_t len = evbuffer_get_length(input);
        if (len < MSG_HEAD_SIZE) {
            return;
        }
        auto headp = (MSG_HEAD*)evbuffer_pullup(input, MSG_HEAD_SIZE);
        const size_t msg_total_len = MSG_HEAD_SIZE + headp->optslen + headp->datalen;
        if (len < msg_total_len) {
            return;
        }
        MSG_HEAD head;
        evbuffer_remove(input, &head, MSG_HEAD_SIZE);
        if (head.version == 1) {
            Close(bev);
            ZLOG_ERROR("version 1 is not support");
            return;
        }
        char* opts = nullptr;
        OptionRequestId opt_req_id;
        if (head.optslen != 0) {
            opts = new char[head.optslen]();
            evbuffer_remove(input, opts, head.optslen);
            opt_req_id = *(OptionRequestId*)opts;
        }

        switch (head.msgtype) {
        case kMtHeartBeat: {  // 心跳包没有消息体，只有option
            evbuffer_drain(input, head.datalen);
            OptionRequestId server_time;
            server_time.option_id = kOptionId_ServerTime;
            server_time.option_buf = time(0);
            head.msgtype = kMtHeartBeatAns;
            head.optslen = sizeof(OptionRequestId);
            bufferevent_write(bev, &head, MSG_HEAD_SIZE);
            bufferevent_write(bev, &server_time, sizeof(OptionRequestId));
            ZLOG_DEBUG("Recv {} Heatbeat", (int64_t)bev);
        } break;

        case kMtSubscribe: {
        } break;
        case kMtUnsubscribe: {           

        } break;


        default:
            ZLOG_ERROR("UNKnow Msg {} Type datalen : {},msgtype {}",
                       (int64_t)bev,
                       head.datalen,
                       head.msgtype);
            evbuffer_drain(input, head.datalen);
            break;
        }

        if (opts != nullptr) {
            delete[] opts;
        }
    }
}

void XTcpServer::OnLoop() {
    // pub_data();
}
}  // namespace linker
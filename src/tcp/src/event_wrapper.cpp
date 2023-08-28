/**
 * @file event_wrapper.cpp
 * @author yangchen
 * @brief event 封装
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "event_wrapper.h"
#include "zlog/ztp_log.h"
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/bufferevent_ssl.h"
#include "event2/buffer.h"
#include "qtp_msg.h"
#include "event2/event.h"
#include "event2/thread.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

namespace tcp {

QtpMsgPtr decode_msg_with_head(struct evbuffer* input) {
    const size_t len = evbuffer_get_length(input);
    if (len < MSG_HEAD_SIZE) {
        return nullptr;
    }
    auto headp = (MSG_HEAD*)evbuffer_pullup(input, MSG_HEAD_SIZE);
    const size_t msg_total_len = MSG_HEAD_SIZE + headp->optslen + headp->datalen;

    if (len < msg_total_len) {
        return nullptr;
    }

    MSG_HEAD head;
    evbuffer_remove(input, &head, MSG_HEAD_SIZE);
    char* opt_buf = nullptr;

    if (head.optslen != 0) {
        opt_buf = new char[head.optslen]();
        evbuffer_remove(input, opt_buf, head.optslen);
    }

    char* data = nullptr;
    if (head.datalen != 0) {
        data = new char[head.datalen];
        evbuffer_remove(input, data, head.datalen);
    }

    return std::make_shared<QtpMsg>(head, data, opt_buf);
}

void TimeoutCallback(evutil_socket_t fd, short what, void* user_data) {
    if (user_data) {
        ((EventWrapper*)user_data)->OnLoop();
        ((EventWrapper*)user_data)->OnExit();
    }
}

struct EventWrapperDataImpl {
    std::mutex mutex_;

    ConnectionParams params_;
    bool use_thread_ = false;

    event_base* event_base_ = nullptr;
    event* default_event_ = nullptr;
    std::atomic_bool is_running_;
};
#define impl ((EventWrapperDataImpl*)pimpl_)

void EventWrapper::ReadCallback(bufferevent* bev, void* user_data) {
    ((EventWrapper*)user_data)->OnRead(bev);
}

void EventWrapper::WriteCallback(bufferevent* bev, void* user_data) {
    ((EventWrapper*)user_data)->OnWrite(bev);
}

void EventWrapper::EventCallback(bufferevent* bev, short events, void* user_data) {
    EventWrapper* base = (EventWrapper*)user_data;

    if (events & BEV_EVENT_EOF || events & BEV_EVENT_ERROR || events & BEV_EVENT_READING ||
        events & BEV_EVENT_WRITING) {
        int errcode = EVUTIL_SOCKET_ERROR();
        ZLOG_TRACE("EventWrapper::EventCallback: {}, events: BEV_EVENT_ERROR {}, errcoder:{}",
                   (void*)bev,
                   events,
                   errcode);
        unsigned long oslerr;
        char buffer[256];
        while (base->params().is_ssl && (oslerr = bufferevent_get_openssl_error(bev))) {
            ERR_error_string_n(oslerr, buffer, sizeof(buffer));
            ZLOG_ERROR("EventWrapper::EventCallback: {}, events: BEV_EVENT_ERROR {}, errcoder:{}, "
                       "oslerr:{} "
                       "error:{}",
                       (void*)bev,
                       events,
                       errcode,
                       oslerr,
                       buffer);
        }
    }
    base->OnEvent(bev, events);
}

int EventWrapper::SetTcpNoDelay(bufferevent* bev) {
    evutil_socket_t fd = bufferevent_getfd(bev);
    int value = 1;
#ifndef _WIN32
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
#else
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&value, sizeof(value));
#endif
}

int EventWrapper::SetKeepAlive(
    bufferevent* bev, int keepAlive, int keepIdle, int keepInterval, int keepCount) {
    evutil_socket_t fd = bufferevent_getfd(bev);
#ifndef _WIN32
    ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
    ::setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    ::setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
    ::setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
#else
    ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive));
#endif
    return 0;
}

EventWrapper::EventWrapper(bool use_thread) {
    pimpl_ = new EventWrapperDataImpl;

    impl->is_running_ = true;
    impl->use_thread_ = use_thread;
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif

    if (use_thread) {
#ifdef _WIN32
        evthread_use_windows_threads();
#else
        evthread_use_pthreads();
#endif
    }

    impl->event_base_ = event_base_new();
    if (impl->event_base_ == nullptr) {
        ZLOG_ERROR("event_base_new return nullptr.");
    }
    if (use_thread) {
        evthread_make_base_notifiable(impl->event_base_);
    }
}

EventWrapper::~EventWrapper() {
    Stop();

    if (pimpl_) {
        delete impl;
        pimpl_ = nullptr;
    }
}

void EventWrapper::OnExit() {
    if (!impl->is_running_) {
        event_del_block(impl->default_event_);
        event_base_loopexit(impl->event_base_, NULL);
    }
}

void EventWrapper::Stop() {
    impl->is_running_ = false;
}

void EventWrapper::Run() {
    if (!impl->event_base_) {
        return;
    }

    impl->default_event_ = event_new(impl->event_base_, -1, EV_PERSIST, TimeoutCallback, this);

    if (impl->default_event_) {
        timeval tv = { 0, 1 };
        event_add(impl->default_event_, &tv);
    }

    event_base_dispatch(impl->event_base_);
    event_base_free(impl->event_base_);
}

event_base* EventWrapper::base() {
    return impl->event_base_;
}

bool EventWrapper::use_thread() const {
    return impl->use_thread_;
}

void EventWrapper::setParams(const ConnectionParams& p) {
    impl->params_ = p;
}

const ConnectionParams& EventWrapper::params() const {
    return impl->params_;
}

ConnectionParams& EventWrapper::params() {
    return impl->params_;
}

}  // namespace tcp

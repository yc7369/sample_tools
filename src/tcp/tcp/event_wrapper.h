/**
 * @file event_wrapper.h
 * @author yangchen
 * @brief event 基础
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef LINKER_LIBEVENT_WRAPPER_H
#define LINKER_LIBEVENT_WRAPPER_H

#include <map>
#include <mutex>
#include <atomic>
#include <netinet/tcp.h>
#include "qtp_msg.h"

#ifdef _WIN32

#ifndef evutil_socket_t
#define evutil_socket_t intptr_t
#endif

#ifndef timeval_t_struct
#define timeval_t_struct
struct timeval_t {
    long tv_sec;  /* seconds */
    long tv_usec; /* and microseconds */
};
#endif
#else
#define timeval_t timeval

#ifndef evutil_socket_t
#define evutil_socket_t int
#endif
#endif

struct bufferevent;
struct event_base;
struct evbuffer;
// struct sockaddr;

namespace tcp {

QtpMsgPtr decode_msg_with_head(struct evbuffer* input);

class EventWrapper {
public:
    /**
     * @brief Construct a new Event Wrapper object
     *
     */
    EventWrapper(bool use_thread = false);
    /**
     * @brief Destroy the Event Wrapper object
     *
     */
    virtual ~EventWrapper();
    /**
     * @brief 读回调
     *
     * @param bev
     * @param user_data
     */
    static void ReadCallback(bufferevent* bev, void* user_data);
    /**
     * @brief 写回调
     *
     * @param bev
     * @param user_data
     */
    static void WriteCallback(bufferevent* bev, void* user_data);
    /**
     * @brief 事件回调
     *
     * @param bev
     * @param events
     * @param user_data
     */
    static void EventCallback(bufferevent* bev, short events, void* user_data);

    /**
     * @brief Set the Tcp No Delay object
     *
     * @param bev
     * @return int
     */
    static int SetTcpNoDelay(bufferevent* bev);

    /**
     * @brief Set the Keep Alive object
     *
     * @param bev
     * @param keepAlive
     * @param keepIdle
     * @param keepInterval
     * @param keepCount
     * @return int
     */
    static int SetKeepAlive(bufferevent* bev,
                            int keepAlive = 1,
                            int keepIdle = 30,
                            int keepInterval = 5,
                            int keepCount = 3);

    /**
     * @brief 启动循环
     *
     */
    void Run();
    /**
     * @brief 停止循环
     *
     */
    void Stop();

    /**
     * @brief 事件读
     *
     * @param bev
     */
    virtual void OnRead(bufferevent* bev){};

    /**
     * @brief 事件写
     *
     * @param bev
     */
    virtual void OnWrite(bufferevent* bev){};
    /**
     * @brief 事件回调
     *
     * @param bev
     * @param events
     */
    virtual void OnEvent(bufferevent* bev, short events){};
    /**
     * @brief Accept 事件
     *
     * @param bev
     * @param fd
     * @param sa
     * @param socklen
     */
    virtual void OnAccept(bufferevent* bev, evutil_socket_t fd, struct sockaddr* sa, int socklen){};
    /**
     * @brief 循环回调
     *
     */
    virtual void OnLoop(){};
    /**
     * @brief event base
     *
     * @return event_base*
     */
    event_base* base();

    virtual void OnExit() final;

    bool use_thread() const;

    void setParams(const ConnectionParams& p);

    const ConnectionParams& params() const;

    ConnectionParams& params();

private:
    void* pimpl_ = nullptr;
};

}  // namespace tcp

#endif  // ZTP_LIBEVENT_WRAPPER_H

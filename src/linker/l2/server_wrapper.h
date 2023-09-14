/**
 * @file server_wrapper.h
 * @author wangjianling (wangjianling)
 * @brief 封装服务端
 * @version 0.1
 * @date 2023-2-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <atomic>
#include <unordered_set>
#include "tcp/event_server.h"
#include "qtp_msg.h"
#include "concurrentqueue.h"

struct Session {
    Session(){};
    Session(bufferevent* b, int64_t se) {
        bev = b;
        id = se;
        // last_req_id = 0;
    }
    bufferevent* bev;
    int64_t id;  // 递增
};

using SessionPtr = std::shared_ptr<Session>;
using SessionSet = std::unordered_set<int64_t>;

inline int64_t get_req_id(QtpMsgPtr msg) {
    auto bev_opt = msg->get_option(kOptionId_ReqId);
    if (bev_opt == nullptr) {
        return 0;
    }
    return *(int64_t*)(bev_opt->option_buf);
}

class ServerEnd : public tcp::EventServer {
public:
    void HandleMsgToClient(QtpMsgPtr qtp_msg);

    void PushMsgToClient(QtpMsgPtr qtp_msg);

protected:
    virtual void onMessage(bufferevent* bev, const QtpMsgPtr qtp_msg) {}

private:
    /**
     * @brief 处理连接请求
     *
     * @param bev
     * @param msg
     */
    void OnAccept(bufferevent* bev, evutil_socket_t fd, struct sockaddr* sa, int socklen) override;
    /**
     * @brief 数据可读回调
     *
     * @param bev
     */
    void OnRead(bufferevent* bev) override;

    /**
     * @brief event 事件回调
     *
     * @param bev
     * @param events
     */
    void OnEvent(bufferevent* bev, short events) override;

    /**
     * @brief 循环回调
     *
     */
    virtual void OnLoop() override;

    /**
     * @brief 处理心跳消息
     *
     * @param bev
     * @param msg
     */
    void HandleBeat(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 处理登录消息
     *
     * @param bev
     * @param msg
     */
    void HandleLogin(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 是否登录
     *
     */
    bool IsLogin(const QtpMsgPtr msg) const;

private:
    moodycamel::ConcurrentQueue<QtpMsgPtr> msg_to_clients_;
    std::unordered_set<bufferevent*> bev_set_;
    SessionSet session_logined_;

    std::atomic_int64_t cur_session_id_ = 0;  // 当前可分配session_id
};

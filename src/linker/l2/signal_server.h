/**
 * @file signal_server.h
 * @author yangdian (yangdian)
 * @brief signal_server 主体
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

#include <unordered_map>
#include <memory>
#include <thread>
#include "server_wrapper.h"
#include "msg.h"
// #include "load_cache.h"
#include "common.h"
// #include "factors.h"

namespace sigsrv {
class SignalServer : public ServerEnd {
public:
    /**
     * @brief Construct a new SignalServer object
     *
     * @param option
     */
    SignalServer() {}
    /**
     * @brief Destroy the SignalServer object
     *
     */
    ~SignalServer();

    /**
     * @brief 初始化
     *
     */
    bool Init();

private:
    virtual void onMessage(bufferevent* bev, const QtpMsgPtr msg) override;

    /**
     * @brief 处理消息
     *
     * @param bev
     * @param msg
     */
    void HandleUnsubscribe(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 处理消息
     *
     * @param bev
     * @param msg
     */
    void HandleSubscribe(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 查询公式
     *
     * @param bev
     * @param msg
     */
    void HandleQueryFormular(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 查询因子
     *
     * @param bev
     * @param msg
     */
    void HandleQueryFactor(bufferevent* bev, const QtpMsgPtr msg);

    /**
     * @brief 因子实例状态操作
     *
     * @param bev
     * @param msg
     */
    void HandleFactorOper(bufferevent* bev, const QtpMsgPtr msg);



    void HandleFactorSupplement(bufferevent* bev, const QtpMsgPtr msg);

    void ResponseMsg(QtpMsgPtr msg, int msg_type, std::string content) {
        if (msg) {
            content.push_back('\0');
            msg->set_msgtype(msg_type);
            msg->set_data(content);
            PushMsgToClient(msg);
        }
    }

private:
};
}  // namespace sigsrv

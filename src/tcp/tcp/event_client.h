/**
 * @file event_client.h
 * @author yangchen
 * @brief libevent 封装
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef LINKER_EVENT_Client
#define LINKER_EVENT_Client
#include "event_wrapper.h"
#include <thread>
#include <memory>

namespace tcp {

class EventClient : public EventWrapper {
public:
    /**
     * @brief 定时器回调函数，用来实现Loop
     *
     * @param fd
     * @param what
     * @param user_data
     */
    static void TimeoutCallback(evutil_socket_t fd, short what, void* user_data);
    /**
     * @brief Construct a new Event Client object
     *
     */
    EventClient(bool use_thread = false);
    /**
     * @brief Destroy the Event Client object
     *
     */
    virtual ~EventClient();
    /**
     * @brief 启动客户端线程
     *
     * @param params
     * @return int
     */
    int Start(const ConnectionParams& params);

    /**
     * @brief 创建SSL context
     *
     * @return SSL*
     */
    void* CreateSSL();

protected:
    /**
     * @brief 停止连接
     *
     * @return int
     */
    int Stop();
    /**
     * @brief 连接TCP
     *
     * @return int
     */
    int Connect();
    /**
     * @brief 关闭TCP连接
     *
     * @return int
     */
    int Close();

    bool isConnected();

    /**
     * @brief 发送数据
     *
     * @param data
     * @param size
     * @return true
     * @return false
     */
    bool Send(const void* data, size_t size);

    /**
     * @brief 重连
     *
     * @return int
     */
    int ReConnect();

protected:
    /**
     * @brief 重连定时器回调
     *
     */
    void OnTimeout();
    /**
     * @brief 连接连上回调
     *
     */
    virtual void OnConnected();
    /**
     * @brief 断开连接回调
     *
     */
    virtual void OnDisconnected() final;
    /**
     * @brief 事件回调
     *
     * @param bev
     * @param events
     */
    virtual void OnEvent(bufferevent* bev, short events) override;

private:
    void* pimpl_ = nullptr;
};
}  // namespace tcp

#endif
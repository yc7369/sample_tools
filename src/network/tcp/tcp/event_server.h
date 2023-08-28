/**
 * @file event_server.h
 * @author yangchen
 * @brief libevent server
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef LINKER_EVENT_SERVER
#define LINKER_EVENT_SERVER
#include "zlog/ztp_log.h"
#include "event_wrapper.h"

namespace tcp {
class EventServer : public EventWrapper {
public:
    /**
     * @brief Construct a new Event Server object
     *
     */
    EventServer(bool use_thread = false);
    /**
     * @brief Destroy the Event Server object
     *
     */
    ~EventServer();
    /**
     * @brief 开始监听
     *
     * @param params
     * @return int
     */
    int Start(const ConnectionParams& params);
    /**
     * @brief 关闭客户端连接
     *
     * @param bev
     * @return int
     */
    int Close(bufferevent* bev);

    /**
     * @brief 创建 SSL
     *
     * @return SSL*
     */
    void* CreateSSL();

private:
    void* pimpl_ = nullptr;
};
}  // namespace tcp

#endif
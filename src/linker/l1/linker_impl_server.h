#ifndef LINKER_H
#define LINKER_H
#include "tcp/event_server.h"
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace linker {
class XTcpServer : public tcp::EventServer {
public:
    /**
     * @brief Construct a new Megate object
     *
     * @param option
     */
    XTcpServer();
    bool init(const std::string& host);

    /**
     * @brief Destroy the Megate object
     *
     */
    ~XTcpServer();

    void start_run_thread();

private:
    virtual void OnLoop() override;

    void pub_data();
    /**
     * @brief Accept 回调
     *
     * @param bev
     * @param fd
     * @param sa
     * @param socklen
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

private:
    std::atomic_bool sub_pub_man_update_ = false;
    std::mutex sub_pub_man_mutex_;

    std::string config_;  // 172.24.16.139:8888
    std::shared_ptr<std::thread> run_thread_;
    std::shared_ptr<std::thread> pub_thread_;
};
}  // namespace linker

#endif
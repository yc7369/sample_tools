#include "server_wrapper.h"
#include <arpa/inet.h>
#include <event2/buffer.h>
#include "zlog/ztp_log.h"

void ServerEnd::OnAccept(bufferevent* bev, evutil_socket_t fd, struct sockaddr* sa, int socklen) {

    int port = ntohs(((sockaddr_in*)sa)->sin_port);
    char ip[128] = { 0 };
    if (sa->sa_family == AF_INET) {
        inet_ntop(sa->sa_family, &((sockaddr_in*)sa)->sin_addr, ip, sizeof(ip));
    } else {
        inet_ntop(sa->sa_family, &((sockaddr_in6*)sa)->sin6_addr, ip, sizeof(ip));
    }

    ZLOG_INFO("Accept {} ip:{} port:{}", (int64_t)bev, ip, port);
    bev_set_.insert(bev);
}

void ServerEnd::OnEvent(bufferevent* bev, short events) {
    if (events == BEV_EVENT_CONNECTED) {
        return;
    }

    if (events & BEV_EVENT_EOF || events & BEV_EVENT_ERROR || events & BEV_EVENT_READING ||
        events & BEV_EVENT_WRITING) {
        ZLOG_ERROR("gateway exception");
        bev_set_.erase(bev);
    }
}

void ServerEnd::OnRead(bufferevent* bev) {
    struct evbuffer* input = bufferevent_get_input(bev);
    while (true) {
        auto msg = tcp::decode_msg_with_head(input);
        if (msg == nullptr) {
            return;
        }

        MSG_HEAD head = msg->head();
        ZLOG_TRACE("head.msgtype: {}", head.msgtype);
        if (head.msgtype == 101) {  // 心跳包没有消息体，只有option
            HandleBeat(bev, msg);
            continue;
        } else if (head.msgtype == 1001) {
            HandleLogin(bev, msg);
            continue;
        }

        if (!IsLogin(msg)) {
            ZLOG_ERROR("not login");
            continue;
        }

        onMessage(bev, msg);
    }
}

void ServerEnd::OnLoop() {
    QtpMsgPtr qtp_msg;
    if (msg_to_clients_.try_dequeue(qtp_msg)) {
        HandleMsgToClient(qtp_msg);
    }
}

void ServerEnd::HandleBeat(bufferevent* bev, const QtpMsgPtr msg) {
    msg->set_msgtype(102);
    msg->add_option(kOptionId_ServerTime, (int64_t)time(0));
    msg->send_data(bev);
}

void ServerEnd::HandleLogin(bufferevent* bev, const QtpMsgPtr msg) {
    if (IsLogin(msg)) {
        ZLOG_INFO("already login");
        msg->set_msgtype(1002);
        msg->send_data(bev);
        return;
    }
    msg->set_msgtype(1002);
    auto session_id = ++cur_session_id_;
    session_logined_.insert(session_id);
    msg->add_option(OptionId::kOptionId_SessionId, session_id);
    msg->send_data(bev);
    ZLOG_TRACE("session_id: {} login", session_id);
}

void ServerEnd::HandleMsgToClient(QtpMsgPtr qtp_msg) {
    if (bev_set_.empty()) {
        ZLOG_ERROR("bev_set_ is empty");
        return;
    }
    // ZLOG_TRACE("send to client,  msgtype: {}", qtp_msg->head().msgtype);
    qtp_msg->send_data(*bev_set_.begin());
}

void ServerEnd::PushMsgToClient(QtpMsgPtr qtp_msg) {
    msg_to_clients_.enqueue(qtp_msg);
}

bool ServerEnd::IsLogin(const QtpMsgPtr msg) const {
    auto session_opt = msg->get_option(OptionId::kOptionId_SessionId);
    if (session_opt == nullptr) {
        ZLOG_ERROR("session id not found");
        return false;
    }

    uint64_t session_id = *(int64_t*)session_opt->option_buf;
    ZLOG_TRACE("head.msgtype: {}, session_id:{}", msg->head().msgtype, session_id);
    if (session_logined_.find(session_id) == session_logined_.end()) {
        ZLOG_ERROR("session id: {} not logged in", session_id);
        return false;
    }
    return true;
}

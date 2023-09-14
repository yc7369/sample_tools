/**
 * @file signal_server.cpp
 * @author yangdian (yangdian)
 * @brief 信号服务主体
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "signal_server.h"
#include "qtp_msg.h"
#include "mybase/timerman.h"
#include "util.h"
#include <type_traits>
#include "common.h"
#include "json.hpp"

using namespace nlohmann;

namespace sigsrv {

SignalServer::~SignalServer() {}

bool SignalServer::Init() {
    return true;
}

void SignalServer::onMessage(bufferevent* bev, const QtpMsgPtr msg) {
    const auto& head = msg->head();
    switch (head.msgtype) {
    case FactorMsgT::kMtCmnSubscribe: {
        HandleSubscribe(bev, msg);
    } break;

    default: {
        ZLOG_INFO("recv req id {}  req", get_req_id(msg));
    } break;
    }
}

void SignalServer::HandleSubscribe(bufferevent* bev, const QtpMsgPtr msg) {
    auto session_opt = msg->get_option(OptionId::kOptionId_SessionId);
    auto session_id = *(int64_t*)session_opt->option_buf;
    ZLOG_INFO("HandleSubscribe session_id:{}", session_id);
    json doc;
    try {
        doc = json::parse(std::string(msg->data(), msg->data_len()));
    } catch (const std::exception& e) {
        ZLOG_ERROR("parse msg failed:{}", e.what());
        return;
    }

    // if (doc.contains("factor_id") && doc["factor_id"].is_string()) {
    //     std::string factor_id = doc["factor_id"];
    //     if (ClientSubMan::getInstance().Subscribe(session_id, factor_id)) {
    //         ResponseMsg(msg, FactorMsgT::kMtCmnSubscribeAns, "success.");
    //         return;
    //     }
    // } else if (doc.contains("formular_id") && doc["formular_id"].is_string()) {
    //     std::string formular_id = doc["formular_id"];
    //     if (ClientSubMan::getInstance().SubscribeFormularId(session_id, formular_id)) {
    //         ResponseMsg(msg, FactorMsgT::kMtCmnSubscribeAns, "success.");
    //         return;
    //     }
    // } else if (doc.contains("formular_name") && doc["formular_name"].is_string()) {
    //     std::string formular_name = doc["formular_name"];
    //     if (ClientSubMan::getInstance().SubscribeFormularName(session_id, formular_name)) {
    //         ResponseMsg(msg, FactorMsgT::kMtCmnSubscribeAns, "success.");
    //         return;
    //     }
    // }
    // ZLOG_ERROR("doc: {}", doc.dump());
    ResponseMsg(msg, FactorMsgT::kMtCmnSubscribeAns, "failed.");
}


}  // namespace sigsrv

#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <set>
#include "tcp/event_client.h"
#include "common.h"
#include "mybase/timerman.h"
#include "concurrentqueue.h"
#include "json.hpp"
#include "qtp_msg.h"

#ifndef MYLOG
#include <iostream>
#define MYLOG(str) std::cout << __FILE__ << ":" << __LINE__ << ":" << str << std::endl;
#endif

using namespace std;
using namespace nlohmann;

static bool readConfigFile(const char* cfgfilepath, const string& key, string& value) {
    fstream cfgFile;
    cfgFile.open(cfgfilepath);  // 打开文件
    if (!cfgFile.is_open()) {
        MYLOG("can not open cfg file!");
        return false;
    }
    char tmp[1000];
    while (!cfgFile.eof())            // 循环读取每一行
    {
        cfgFile.getline(tmp, 1000);   // 每行读取前1000个字符，1000个应该足够了
        string line(tmp);
        size_t pos = line.find('=');  // 找到每行的“=”号位置，之前是key之后是value
        if (pos == string::npos)
            return false;
        string tmpKey = line.substr(0, pos);  // 取=号之前
        if (key == tmpKey) {
            value = line.substr(pos + 1);     // 取=号之后
            return true;
        }
    }
    return false;
}

std::string read_file(const std::string& file) {
    std::ifstream ifs(file);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return content;
}

class TestClient : public tcp::EventClient {
public:
    void SendMsg(QtpMsg& msg) {
        auto head = msg.head();
        Send(&head, sizeof(MSG_HEAD));
        if (head.optslen > 0) {
            Send(msg.options(), head.optslen);
        }
        if (head.datalen > 0) {
            Send(msg.data(), head.datalen);
        }
    }

    void SendHeartBeat() {
        QtpMsg msg;
        msg.set_msgtype(101);
        msg_queue_.enqueue(msg);
    }

    void SendLogin() {
        QtpMsg msg;
        msg.set_msgtype(FactorMsgT::kMtCmnLogin);
        msg_queue_.enqueue(msg);
    }

    void SendOnMsgType(uint16_t msg_type, const std::string& data = "") {
        QtpMsg msg;
        msg.set_msgtype(msg_type);
        msg.add_option(OptionId::kOptionId_SessionId, session_id_);
        if (!data.empty()) {
            msg.set_data(data);
        }
        if (msg_type == FactorMsgT::kMtFactorCalc) {
            auto xx = read_file("sample_cal.json");
            std::string data = xx;

            msg.set_data(data);
            msg.add_option(OptionId::kOptionId_ReqId, 888888);
            msg.add_option(OptionId::kOptionId_ServerTime, 123456);
        } else if (msg_type == FactorMsgT::kMtCmnSubscribe) {
            // std::string data = R"({
            //     "factor_id": 1
            // })";
            // std::string data = R"({
            //     "formular_id": 4
            // })";
            std::string data = R"({
                "formular_name": "CashflowZSpread"
            })";
            msg.set_data(data);
        } else if (msg_type == FactorMsgT::kMtCmnUnsubscribe) {
            // std::string data = R"({
            //     "factor_id": 1
            // })";
            // std::string data = R"({
            //     "formular_id": 4
            // })";
            std::string data = R"({
                "formular_name": "CashflowZSpread000"
            })";
            msg.set_data(data);
        } else if (msg_type == FactorMsgT::kMtFactorConf) {
            auto config = read_file("factors_mass.json");
            config.push_back('\0');
            json config_doc = json::parse(config);
            json req_doc;
            req_doc["cmd"] = "cover";  // 覆盖所有配置
            req_doc["config"] = config_doc;
            MYLOG("req_doc:" << req_doc.dump());
            msg.set_data(config);
        } else if (msg_type == FactorMsgT::kMtCmnClientDisconnected) {
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "{\"session_id\":%ld}", session_id_);
            msg.set_data(buffer);
            session_id_ = 0;
        }

        msg.encode();
        MYLOG("msg optionlen:" << msg.opt_len());
        msg_queue_.enqueue(msg);
    }

    virtual void OnRead(bufferevent* bev) override {
        struct evbuffer* input = bufferevent_get_input(bev);
        while (true) {

            auto msg = tcp::decode_msg_with_head(input);

            if (msg == nullptr) {
                return;
            }

            MSG_HEAD head = msg->head();
            MYLOG("recv msg:" << head.msgtype);
            if (head.msgtype == FactorMsgT::kMtCmnLoginAns) {
                MYLOG("recv msg:" << head.msgtype);
                auto session_opt = msg->get_option(OptionId::kOptionId_SessionId);
                session_id_ = *(int64_t*)session_opt->option_buf;
                MYLOG("session_id: " << session_id_);
            } else if (head.msgtype == FactorMsgT::kMtFactorCalcAns ||
                       head.msgtype == FactorMsgT::kMtFactorConfAns ||
                       head.msgtype == FactorMsgT::kMtCmnQueryFormularAns ||
                       head.msgtype == FactorMsgT::kMtCmnQueryFactorAns ||
                       head.msgtype == FactorMsgT::kMtCmnUnsubscribeAns ||
                       head.msgtype == FactorMsgT::kMtCmnSubscribeAns ||
                       head.msgtype == FactorMsgT::kMtModifyFormularTypeAns ||
                       head.msgtype == FactorMsgT::kMtQueryFormularTypeAns ||
                       head.msgtype == FactorMsgT::kMtCmnFactorOperAns ||
                       head.msgtype == FactorMsgT::kMtCmnQueryFormularTreeAns ||
                       head.msgtype == FactorMsgT::kMtCmnFactorSupplementAns) {
                MYLOG("recv msg:" << head.msgtype);
                auto req_id_opt = msg->get_option(OptionId::kOptionId_ReqId);
                uint64_t req_id = 0;
                if (req_id_opt) {
                    req_id = *(int64_t*)req_id_opt->option_buf;
                }
                auto server_time_opt = msg->get_option(OptionId::kOptionId_ServerTime);
                uint64_t server_time = 0;
                if (server_time_opt) {
                    server_time = *(int64_t*)server_time_opt->option_buf;
                }
                MYLOG("OptionId::kOptionId_ReqId:" << req_id << ", OptionId::kOptionId_UserToken:"
                                                   << server_time);
                MYLOG("recv msg:" << std::string(msg->data(), msg->data_len()));
            } else if (head.msgtype == FactorMsgT::kMtCmnPublish) {
                auto req_id_opt = msg->get_option(OptionId::kOptionId_ReqId);
                uint64_t req_id = 0;
                if (req_id_opt) {
                    req_id = *(int64_t*)req_id_opt->option_buf;
                }
                auto server_time_opt = msg->get_option(OptionId::kOptionId_ServerTime);
                uint64_t server_time = 0;
                if (server_time_opt) {
                    server_time = *(int64_t*)server_time_opt->option_buf;
                }
                MYLOG("OptionId::kOptionId_ReqId:" << req_id << ", OptionId::kOptionId_UserToken:"
                                                   << server_time);
                MYLOG("recv msg:" << std::string(msg->data(), msg->data_len()));
            }
        }
    }

    virtual void OnLoop() override {
        std::time_t now = mybase::time::unixTimeNowMs() / 1000;
        if (now - last_heart_beat_time_ > 3) {
            SendOnMsgType(101, "");
            last_heart_beat_time_ = now;
        }
        QtpMsg msg;
        while (msg_queue_.try_dequeue(msg)) {
            MYLOG("send msg:" << msg.msgtype());
            SendMsg(msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void OnConnected() {
        MYLOG("OnConnected");
    }

public:
    void RawEnqueue(u_int16_t num, std::string body) {
        body.push_back('\0');

        QtpMsg msg;
        msg.add_option(OptionId::kOptionId_SessionId, session_id_);
        msg.add_option(OptionId::kOptionId_ReqId, 888888);

        msg.set_msgtype(num);
        msg.set_data(body);
        msg.encode();
        MYLOG("msg optionlen:" << msg.opt_len());
        msg_queue_.enqueue(msg);
    }

private:
    uint64_t session_id_ = 0;
    std::time_t last_heart_beat_time_ = 0;

    moodycamel::ConcurrentQueue<QtpMsg> msg_queue_;
};

static void TestUseFile(TestClient& cli) {

   std::ifstream file("case.xml");

    // 检查文件是否成功打开
    if (!file.is_open()) {
        std::cerr << "无法打开文件: case.xml"<< std::endl;
        return ;
    }

    std::string line;
    std::vector<std::string> jsonLines;
    bool foundDelimiter = false;

    int num = 0;
    // 逐行读取文件内容
    if(std::getline(file, line)){
        num = atoi(line.c_str());
        if(num == 0){
            std::cout<<"input msgtype error"<<std::endl;
            return ;
        }
    }
    while (std::getline(file, line)) {
        // 如果找到分隔符 "|||"，则从下一行开始提取JSON数据
        if (foundDelimiter) {
            // 在这里可以处理JSON数据，例如打印
            jsonLines.push_back(line);
        }

        // 检查是否找到分隔符
        if (line == "|||") {
            foundDelimiter = true;
        }
    }

    // 关闭文件
    file.close();

    // 合并JSON数据行
    std::string jsonData = "";
    for (const std::string& jsonLine : jsonLines) {
        jsonData += jsonLine;
    }

    cli.RawEnqueue((u_int16_t)num, jsonData);
}

int main() {
    std::string ip;
    if (!readConfigFile("config.ini", "ip", ip)) {
        MYLOG("read ip error");
    }
    std::string port;
    if (!readConfigFile("config.ini", "port", port)) {
        MYLOG("read port error");
    }

    MYLOG("ip:" << ip << ", port:" << port);

    TestClient client;
    tcp::ConnectionParams conn_param;
    conn_param.is_ssl = 0;
    conn_param.address = ip;
    conn_param.port = port;
    client.Start(conn_param);

    mybase::TimerMan::instance()->run();

    // 支持的命令
    std::set<uint16_t> cmdList{ kMtCmnLogin,
                                kMtCmnSubscribe,
                                kMtCmnUnsubscribe,
                                kMtCmnQueryFormular,
                                kMtCmnQueryFactor,
                                kMtFactorCalc,
                                kMtFactorConf,
                                kMtModifyFormularType,
                                kMtQueryFormularType,
                                kMtCmnQueryFormularTree,
                                kMtCmnClientDisconnected };

    while (true) {
        char str[120]{};
        std::cin.getline(str, 120);
        uint16_t msg_type = atoi(str);

        if (!*str)
            continue;

        // 测试接口
        if (*(int16_t*)"=" == *(int16_t*)str) {
            TestUseFile(client);
            continue;
        }

        if (cmdList.find(msg_type) == cmdList.end()) {

            std::cout << "Number of commands supported: " << cmdList.size() << std::endl;
            std::cout << "    ";
            for (auto ele : cmdList) {
                std::cout << MessageFactorMsgT(ele) << ": " << ele << "    ";
            }
            std::cout << std::endl;

            continue;
        }
        client.SendOnMsgType(msg_type);
    }

    return 0;
}

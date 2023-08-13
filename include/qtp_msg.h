
#ifndef QTP_MSG_H
#define QTP_MSG_H

#include <vector>
#include <stdint.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <string.h>  //memecpy

#ifdef USE_BUFFEVENT_DEP
#include <event2/event.h>
#include <event2/bufferevent.h>
#endif

namespace tcp {

struct ConnectionParams {
    std::string address;
    std::string port;
    std::string name;          //服务名称
    std::string hostkey;       // etcd key
    std::string etcd_address;  // 写入etcd 的ip
    std::string etcd_port;     //写入etcd的端口
    std::string etcd_endpoints;

    bool is_ssl = false;
    bool cert_verify = false;
    std::string cert_path = "server/";
};

};  // namespace tcp

#pragma pack(push)
#pragma pack(1)
// copy from ztp_event
enum OptionId {
    kOptionId_Reserved = 60000,          // for system event option
    kOptionId_SessionId = 60001,         // user data option
    kOptionId_PageInfo = 60002,          // 为了消息分包消息使用，结构为 PageInfo
    kOptionId_SubscribeKey = 60003,      // 为了订阅的消息使用
    kOptionId_ServerTime = 60004,        // 服务器时间，秒
    kOptionId_ReqId = 60005,             // 请求ID
    kOptionId_UserToken = 600011,        // 用户token
    kOptionId_KLinePeriodType = 600020,  // k线周期
    kOptionId_Extended = 61000,          // for third-party system module

};

struct BufHead {
    size_t count = 0;
    size_t head = 0;
};

enum MSG_VER {
    QTP_VER = 0,
    PB_VER = 1,
    NEVER_USERD = 0x6A,  //大于这个的给交易组用
};

//网关与客户端交互的消息
//采用定长结构
// // 内存格式：option_id | option_size | option_buf
struct MSG_HEAD {
    uint8_t version = 0;
    uint8_t service = 0;
    uint16_t msgtype = 0;
    uint16_t topic = 0;
    uint16_t optslen = 0;
    uint32_t datalen = 0;
};

// struct MSG_HEAD_PB {
//     uint8_t version = PB_VER;
//     uint8_t msgtype = 0;
//     uint16_t datalen = 0;
//     int32_t msginfo = 0;
// };

using MSG_HEAD_PB = MSG_HEAD;

#define MSG_HEAD_SIZE sizeof(MSG_HEAD)
#define MSG_HEAD_PB_SIZE sizeof(MSG_HEAD_PB)

struct PageInfo {
    int32_t total_cnt;   // 总项目数
    int16_t total_page;  // 总页数
    int16_t cur_page;    // 当前页数
    int16_t page_size;   // 当前页大小
};

//内存格式：option_id | option_size | option_buf

class OptionRequest {
public:
    ~OptionRequest() {
        if (option_buf != nullptr) {
            delete[] option_buf;
            option_buf = nullptr;
        }
    }
    OptionRequest(){};
    OptionRequest(uint16_t id, uint16_t size, void* data) {
        option_buf = new char[size];
        memcpy(option_buf, data, size);
        option_id = id;
        option_size = size;
    }
    uint16_t option_id = 0;
    uint16_t option_size = 0;
    char* option_buf = nullptr;
};

using OptionRequestPtr = std::shared_ptr<OptionRequest>;

/**
 * @brief 非面对高频数据设计
 *
 */
class QtpMsg {

public:
    ~QtpMsg() {
        reset();
    }

    QtpMsg() {}
    QtpMsg(const MSG_HEAD& head, char* data, char* opt)
        : head_(head), data_len_(head.datalen), data_buf_(data), opt_len_(head.optslen),
          opt_buf_(opt) {
        parse_options();
    }

    QtpMsg(const QtpMsg& msg) {
        set_head(msg.head_);
        set_options(msg.opt_buf_, msg.opt_len_);
        set_data(msg.data_buf_, msg.data_len_);
        parse_options();
    };

    QtpMsg(QtpMsg&& msg)
        : head_(msg.head_), data_len_(msg.data_len_), data_buf_(msg.data_buf_),
          opt_len_(msg.opt_len_), opt_buf_(msg.opt_buf_) {
        msg.head_.datalen = 0;
        msg.head_.optslen = 0;
        msg.data_buf_ = nullptr;
        msg.data_len_ = 0;
        msg.opt_buf_ = nullptr;
        msg.opt_len_ = 0;
        parse_options();
    }

    QtpMsg& operator=(const QtpMsg& msg) {
        set_head(msg.head_);
        set_options(msg.opt_buf_, msg.opt_len_);
        set_data(msg.data_buf_, msg.data_len_);
        parse_options();
        return *this;
    }

    QtpMsg& operator=(QtpMsg&& msg) {
        head_ = msg.head_;
        opt_buf_ = msg.opt_buf_;
        opts_ = msg.opts_;
        data_buf_ = msg.data_buf_;
        data_len_ = msg.data_len_;
        opt_len_ = msg.opt_len_;
        msg.head_.datalen = 0;
        msg.head_.optslen = 0;
        msg.data_buf_ = nullptr;
        msg.data_len_ = 0;
        msg.opt_buf_ = nullptr;
        msg.opt_len_ = 0;
        parse_options();
        return *this;
    };

    void parse_options() {
        size_t offset = 0;
        while (offset < opt_len_) {
            auto opt = (OptionRequest*)(opt_buf_ + offset);
            OptionRequestPtr requset = std::make_shared<OptionRequest>(
                opt->option_id, opt->option_size, (opt_buf_ + offset + sizeof(uint16_t) * 2));
            offset += sizeof(uint16_t) * 2 + requset->option_size;
            opts_[requset->option_id] = requset;
        }
    };

    /**
     * @brief 移除某个option
     *
     * @param option_id
     * @return bool
     */
    bool remove_option(int64_t option_id, OptionRequestPtr request) {
        for (auto it = opts_.begin(); it != opts_.end(); it++) {
            if (it->second->option_id == option_id) {
                request = it->second;
                opts_.erase(it);
                return true;
            }
        }
        return false;
    }

    bool remove_option(int64_t option_id) {
        OptionRequestPtr request;
        return remove_option(option_id, request);
    }

    void add_option(OptionRequestPtr request) {
        opts_[request->option_id] = request;
        encode();
    }

    void add_option(uint16_t id, uint16_t size, void* data) {
        OptionRequestPtr req = std::make_shared<OptionRequest>(id, size, data);
        add_option(req);
    }

    void add_option(uint16_t id, int64_t data) {
        OptionRequestPtr req = std::make_shared<OptionRequest>(id, sizeof(int64_t), &data);
        add_option(req);
    }

    OptionRequestPtr get_option(int64_t option_id) {
        if (opts_.find(option_id) == opts_.end()) {
            return nullptr;
        }
        return opts_[option_id];
    }

    MSG_HEAD head() {
        return head_;
    }

    void set_head(MSG_HEAD h) {
        head_ = h;
    }

    void set_msgtype_topic(uint16_t msg_type, uint16_t topic) {
        head_.msgtype = msg_type;
        head_.topic = topic;
    }

    void set_msgtype(uint16_t msg_type) {
        head_.msgtype = msg_type;
    }
    uint16_t msgtype() {
        return head_.msgtype;
    }
    uint16_t topic() {
        return head_.topic;
    }

    void set_topic(uint16_t topic) {
        head_.topic = topic;
    }

    void set_version(uint8_t version) {
        head_.version = version;
    }

    void reset_body() {
        data_len_ = 0;
        if (data_buf_ != nullptr) {
            delete[] data_buf_;
            data_buf_ = nullptr;
        }
    }
    void reset_option() {
        opt_len_ = 0;
        if (opt_buf_ != nullptr) {
            delete[] opt_buf_;
            opt_buf_ = nullptr;
        }
        opts_.clear();
    }

    void reset() {
        reset_body();
        reset_option();
        head_ = { 0 };
    }

    bool set_data(const std::string& data) {
        return set_data(data.c_str(), data.length());
    }
    bool set_data(const void* data, size_t size) {
        if (data == nullptr) {
            reset_body();
            return false;
        }

        if (data_len_ < size) {
            delete[] data_buf_;
            data_buf_ = new char[size];
        }
        head_.datalen = size;
        data_len_ = size;
        memcpy(data_buf_, data, size);
        return true;
    }

    char* data() {
        return data_buf_;
    }
    size_t data_len() {
        return data_len_;
    }

    char* options() {
        return opt_buf_;
    }

    bool set_options(const void* data, size_t size) {
        if (data == nullptr) {
            return false;
        }

        if (opt_len_ < size) {
            delete[] opt_buf_;
            opt_buf_ = new char[size];
        }

        opt_len_ = size;
        memcpy(opt_buf_, data, size);
        parse_options();
        return true;
    }

    size_t opt_len() {
        return opt_len_;
    }

    uint8_t version() {
        return head_.version;
    }

#ifdef USE_BUFFEVENT_DEP
    bool send_data(bufferevent* bev) {
        encode();

        head_.optslen = opt_len_;
        head_.datalen = data_len_;

        bufferevent_write(bev, &head_, MSG_HEAD_SIZE);

        if (opt_len_ && opt_buf_) {
            bufferevent_write(bev, opt_buf_, opt_len_);
        }
        if (data_len_ && data_buf_) {
            bufferevent_write(bev, data_buf_, data_len_);
        }
        return true;
    }
#endif
    void encode() {
        auto old_opt_len = opt_len_;
        opt_len_ = 0;
        for (auto i : opts_) {
            opt_len_ += i.second->option_size + sizeof(uint16_t) * 2;
        }
        if (old_opt_len < opt_len_) {
            delete[] opt_buf_;
            opt_buf_ = new char[opt_len_];
        }
        encode_opts();
        head_.optslen = opt_len_;
        head_.datalen = data_len_;
    }

private:
    void encode_opts() {
        size_t offset = 0;
        for (auto i : opts_) {
            memcpy(opt_buf_ + offset, &(i.second->option_id), sizeof(uint16_t));
            memcpy(
                opt_buf_ + offset + sizeof(uint16_t), &(i.second->option_size), sizeof(uint16_t));
            memcpy(opt_buf_ + offset + sizeof(uint16_t) * 2,
                   i.second->option_buf,
                   i.second->option_size);
            offset += sizeof(uint16_t) * 2 + i.second->option_size;
        }
    }

private:
    MSG_HEAD head_;
    std::unordered_map<int64_t, OptionRequestPtr> opts_;
    size_t data_len_ = 0;
    char* data_buf_ = nullptr;
    size_t opt_len_ = 0;
    char* opt_buf_ = nullptr;
};

using QtpMsgPtr = std::shared_ptr<QtpMsg>;

#pragma pack(pop)

#endif
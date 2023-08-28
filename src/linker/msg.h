#pragma once
#include <stdint.h>
namespace linker {

#pragma pack(push)
#pragma pack(1)
//内存格式：option_id | option_size | option_buf
struct OptionRequestId {
    uint16_t option_id = kOptionId_ReqId;
    uint16_t option_size = sizeof(int64_t);
    int64_t option_buf = 0;
};


enum SDSMsgType {
    kMtHeartBeat = 101,              // 心跳查询包
    kMtHeartBeatAns = 102,           // 心跳回报
    kMtLoginMegate = 103,            // 登录网关
    kMtLoginMegateAns = 104,         // 登录回报
    kMtSubscribe = 105,              // 订阅行情，[Topic = SDSDataType] [body = hjcode_list]
    kMtUnsubscribe = 106,            // 取消订阅，[Topic = SDSDataType] [body = hjcode_list]
    kMtPublish = 107,                // 发布消息，[Topic = SDSDataType] [body = hjcode_list]
    kMtQueryCode = 108,              // 查询最新行情
    kMtQueryCodeAns = 109,           // 查询最新行情回报
    kMtQueryKLine = 110,             // 查询KLine数据
    kMtQueryKLineAns = 111,          // 查询KLine数据回报
    kMtGetSecuMaster = 112,          // 获取所有secumaster基础数据【deprecated】
    kMtGetSecuMasterAns = 113,       // 获取所有secumaster基础数据回报【deprecated】
    kMtMegatePing = 114,             // ping网关
    kMtMegatePong = 115,             // 网关回复
    kMtGetSecuMasterPrice = 116,     // 获取所有secumaster 价格基础数据【deprecated】
    kMtGetSecuMasterPriceAns = 117,  // 获取所有secumaster 价格回报【deprecated】
    kMtGetSecuMasterV2 = 118,        // 获取所有secumaster v2 价格基础数据 [推荐使用]
    kMtGetSecuMasterV2Ans = 119,  // 获取所有secumaster  v2 价格基础数据回报 [推荐使用]
    kMtQueryRTHistory = 120,      // 查询当日历史数据
    kMtQueryRTHistoryAns = 121,  // 查询当日历史数据回报
    kMtSubscribeAns = 122,       // 订阅行情回报
    kMtUnSubscribeAns = 123,     // 取消订阅行情回报
    kMtGetSecuMasterFullist = 124,  // 获取所有secumaster的完整列表代码（sys_ukey表基础字段）
    kMtGetSecuMasterFullistAns = 125,  // 获取所有secumaster的完整列表代码（sys_ukey表基础字段）
};


// 登录请求
struct LoginRequest {
    char acc[16];  // 账户名称
    char pwd[24];  // 密码
    char mac[24];  // mac地址
};

// 登录回报,未登录回报
struct LoginAns {
    int8_t ret;  // 0 成功，其他失败
};

struct PingRequest {
    int64_t time_stamp;  // 客户端发过来的时间戳
};

struct PingAns {
    int64_t time_stamp;  // 客户端发过来的时间戳,原样返回
};
};

#pragma pack(pop)
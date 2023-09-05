#pragma once

#include <string>
#include <memory>



#define TOPIC_ENUM(str) constexpr static const char* str = #str;
struct FactorTopics {
    // 因子配置变化
    // key: #NOUSE
    TOPIC_ENUM(FACTOR_CONFIG_CHANGED)
    // 实时因子数据生产
    TOPIC_ENUM(FACTOR_DATA_REALTIME_CALCULATED)
    // 历史因子数据生产
    TOPIC_ENUM(FACTOR_DATA_HISTORY_CALCULATED)
    // 参数变化
    TOPIC_ENUM(FACTOR_VARIBLES_CHANGED)
};
#undef TOPIC_ENUM

/*! \class 应用消息定义
 *
 */
enum FactorMsgT {
    kMtCmnServiceStatus = 1000,
    kMtCmnLogin = 1001,               // 登录
    kMtCmnLoginAns = 1002,            // 登录应答
    kMtCmnSubscribe = 1003,           // 订阅
    kMtCmnSubscribeAns = 1004,        // 订阅应答
    kMtCmnPublish = 1005,             // 发布
    kMtCmnUnsubscribe = 1006,         // 取消订阅
    kMtCmnUnsubscribeAns = 1007,      // 取消订阅应答
    kMtCmnQueryFormular = 1008,       // 查询公式
    kMtCmnQueryFormularAns = 1009,    // 查询公式应答
    kMtCmnQueryFactor = 1010,         // 查询因子
    kMtCmnQueryFactorAns = 1011,      // 查询因子应答
    kMtCmnFactorOper = 1012,          // 因子实例状态操作
    kMtCmnFactorOperAns = 1013,       // 因子实例状态操作应答
    kMtCmnFactorSupplement= 1014,     // 因子数据补录
    kMtCmnFactorSupplementAns= 1015,  // 因子数据补录应答

    kMtCmnClientDisconnected = 1099,  // 客户端断线

    kMtFactorCalc = 2000,
    kMtFactorCalcAns = 2001,
    kMtFactorConf = 2002,
    kMtFactorConfAns = 2003,
    kMtModifyFormularType = 2004,       // 修改公式类别
    kMtModifyFormularTypeAns = 2005,    // 修改公式类别应答
    kMtQueryFormularType = 2006,        // 查询公式类别
    kMtQueryFormularTypeAns = 2007,     // 查询公式类别应答
    kMtCmnQueryFormularTree = 2008,     // 查询公式依赖树
    kMtCmnQueryFormularTreeAns = 2009,  // 查询公式依赖树应答
};

constexpr const char* MessageFactorMsgT(int item) {
    switch (item) {
#define XXX(type) \
    case type:    \
        return #type
        XXX(kMtCmnServiceStatus);
        XXX(kMtCmnLogin);
        XXX(kMtCmnLoginAns);
        XXX(kMtCmnSubscribe);
        XXX(kMtCmnSubscribeAns);
        XXX(kMtCmnPublish);
        XXX(kMtCmnUnsubscribe);
        XXX(kMtCmnUnsubscribeAns);
        XXX(kMtCmnQueryFormular);     // 查询公式
        XXX(kMtCmnQueryFormularAns);  // 查询公式应答
        XXX(kMtCmnQueryFactor);       // 查询因子
        XXX(kMtCmnQueryFactorAns);    // 查询因子应答
        XXX(kMtCmnFactorSupplement);
        XXX(kMtCmnFactorSupplementAns);



        
        XXX(kMtCmnClientDisconnected);

        XXX(kMtFactorCalc);
        XXX(kMtFactorCalcAns);
        XXX(kMtFactorConf);
        XXX(kMtFactorConfAns);
        XXX(kMtModifyFormularType);
        XXX(kMtModifyFormularTypeAns);
        XXX(kMtQueryFormularType);
        XXX(kMtQueryFormularTypeAns);
        XXX(kMtCmnQueryFormularTree);
        XXX(kMtCmnQueryFormularTreeAns);
#undef XXX

    default:
        return "MessageFactorMsgT Not Found";
    }
}

/*! \class 应用错误码定义
 *
 */
enum ErrorCodeT {
    kErrorNone = 0,
    //
    // 服务不在线
    kErrorServiceOffline = 10000,
    kErrorPyvmNotReady = 10001,
};

inline std::string mapMsgType(FactorMsgT typ) {
    switch (typ) {
    case kMtFactorCalc: {
        return std::to_string(kMtFactorCalc);
    }
    case kMtFactorConf: {
        return std::to_string(kMtFactorConf);
    }
    case kMtModifyFormularType: {
        return std::to_string(kMtModifyFormularType);
    }
    case kMtQueryFormularType: {
        return std::to_string(kMtQueryFormularType);
    }
    case kMtCmnQueryFormularTree: {
        return std::to_string(kMtCmnQueryFormularTree);
    }
    default:
        return "";
    };
    return "";
}

inline std::string mapErrorCode(ErrorCodeT err) {
#define ERROR_STR(err) \
    case err:          \
        return #err;
    switch (err) {
        ERROR_STR(kErrorServiceOffline);
        ERROR_STR(kErrorNone);
    default:
        return "";
    };
    return "";
}

#ifndef DEFINE_SHARED_PTR
#define DEFINE_SHARED_PTR(DataType) using DataType##Ptr = std::shared_ptr<DataType>
#endif

#ifndef DECLARE_DEFINE_SHARED_PTR
#define DECLARE_DEFINE_SHARED_PTR(DataType) \
    class DataType;                         \
    DEFINE_SHARED_PTR(DataType);
#endif

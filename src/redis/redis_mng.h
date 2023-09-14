/*************************************************************
// Created by yangchen on 7/25/20.

File Name：文件名称
Function List：函数功能列表，多列注释
函数名称1：功能描述
函数名称2：功能描述
Class：功能描述


**************************************************************/

#ifndef LH_FRAMEWORK_REDIS_MNG_H
#define LH_FRAMEWORK_REDIS_MNG_H

#include "redis/redis++.h"
#include "utils/singleton.h"
#include <atomic>
#include <queue>
using namespace sw::redis;
namespace lhserver {
// redis哨兵配置
typedef struct {
    std::string host;    //主机地址
    int port;            //主机端口
    std::string passwd;  //密码
} SentinelConfig;
typedef std::shared_ptr<SentinelConfig> SentinelConfigPtr;

#define MAX_DB_SIZE 16

using SentinelConfigArray = std::vector<SentinelConfigPtr>;

// redis配置
typedef struct {
    //如果不使用哨兵，则需使用
    std::string host;    //主机地址
    int port;            //主机端口

    //以下字段是必须配置
    std::string passwd;       //密码
    std::string master_name;  //主库名称
    int db;                   //默认的数据库实例id
    int connect_timeout;      //连接超时时间 毫秒
    int socket_timeout;       //请求超时时间 毫秒
    int pool_size;            //连接池大小
} RedisConfig;
typedef std::shared_ptr<RedisConfig> RedisConfigPtr;
typedef std::shared_ptr<Redis> RedisPtr;
typedef struct {
    bool master;
    RedisPtr redis;
}RealRedis;
typedef std::shared_ptr<RealRedis> RealRedisPtr;
typedef std::shared_ptr<Sentinel> SentinelPtr;

// redis连接池配置
struct RedisPool {
    int totalsize;                     //当前总的连接数
    int usesize;                       //已经使用的连接数
    int idlesize;                      //闲置的连接数
    std::mutex mtx;                    //连接池锁
    std::list<RealRedisPtr> redis_queue;  // redis连接队列
};
typedef std::shared_ptr<RedisPool> RedisPoolPtr;

// redis连接池管理类
class  RedisMng : public Singleton<RedisMng> {
   public:
    /**
     * @brief 初始化连接池
     *
     * @param[in] sentinel_configs   哨兵配置
     * @param[in] redis_config       redis连接配置
     * @param[out] 无
     * @return 无
     */
    void Init(SentinelConfigArray sentinel_configs, RedisConfigPtr redis_config);

    /**
     * @brief 获取一个有效的redis连接
     *
     * @param[in] master 是否为主库，true 是 false否
     * @param[in] db 数据库实例id，如果为-1则读取配置文件中的db
     * @param[out] 无
     * @return redis连接实例
     */
    RedisPtr GetRedis(bool master = true, int db = -1);

    /**
     * @brief 归还一个redis连接
     *
     * @param[in] redis 通过GetRedis获取的redis连接
     * @param[in] master 是否为主库，true 是 false否
     * @param[in] isvalid
     * 是否有效，如果调用的过程中出现异常则范围的时候为false，如果正常则为true
     * @param[in] db 数据库实例id，次出必须与GetRedis中的db一致
     * @param[out] 无
     * @return 0成功 非0失败
     */
    int GiveBack(RedisPtr redis, bool master, bool isvalid = true, int db = -1);

   private:
    /**
     * @brief 获取一个有效的redis连接,这里针对直连redis
     *
     * @param[in] master 是否为主库，true 是 false否
     * @param[in] db 数据库实例id，如果为-1则读取配置文件中的db
     * @param[out] 无
     * @return redis连接实例
     */
    RedisPtr GetRedis2(bool master = true, int db = -1);

    /**
     * @brief 获取一个连接池
     *
     * @param[in] idx 连接池id
     * @param[out] 无
     * @return 连接池实例
     */
    RedisPoolPtr GetRedisPool(int idx);

    SentinelConfigArray sentinel_configs_;
    RedisConfigPtr redis_config_;
    SentinelPtr sentinel_;

    std::mutex redis_pool_mtx_;
    std::vector<RedisPoolPtr> redis_conn_pool_;
};

// redis连接池代理类
class  RedisMngProxy {
   public:
    /**
     * @brief构造函数
     *
     * @param[in] master 是否为主库
     * @param[in] db redis数据库实例序号
     */
    RedisMngProxy(bool master = true, int db = -1);

    /**
     * @brief析构函数
     */
    ~RedisMngProxy();

    /**
     * @brief
     * 将状态置为无效，redis对象判断为空或者redis操作过程中抛出异常时使用
     */
    void SetInvalid();

    /**
     * @brief 重载 ->操作符，实现对RedisPtr的调用
     * @return RedisPtr redis连接对象
     */
    RedisPtr operator->() const;

    /**
     * @brief 重载 void* 返回原始指针，用于判空
     *
     * @return 返回ptr_的原始指针
     */
    operator void*() const;

    /**
     * @brief 重载 !=
     *
     * @param[in]
     * @param[out]
     * @return 不等于v返回true 等于v返回false
     */
    bool operator!=(const RedisMngProxy& v) const;

    /**
    * @brief 重载 == 
    *
    * @param[in] 
    * @param[out] 
    * @return 等于v返回true 不等于v返回false
    */
    bool operator==(const RedisMngProxy& v) const;

    /**
    * @brief 重载 ==  判空
    *
    * @param[in] 
    * @param[out] 
    * @return 等于nullptr返回true，不等于nullptr返回false
    */
    bool operator==(nullptr_t) const;

    /**
    * @brief 重载 !=  判空
    *
    * @param[in] 
    * @param[out] 
    * @return 不等于nullptr返回true，等于nullptr返回false
    */
    bool operator!=(nullptr_t) const;
private:
    bool isvalid_;
    bool master_;
    int db_;
    RedisPtr ptr_ = nullptr;
};

}  // namespace lhserver
#endif //LH_FRAMEWORK_REDIS_MNG_H

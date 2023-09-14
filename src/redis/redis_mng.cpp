/*************************************************************
// Created by zhujinhua on 7/25/20.

File Name：文件名称
Function List：函数功能列表，多列注释
函数名称1：功能描述
函数名称2：功能描述
Class：功能描述


**************************************************************/

#include "redis_mng.h"

namespace lhserver
{
    void RedisMng::Init(SentinelConfigArray sentinel_configs, RedisConfigPtr redis_config)
    {
        sentinel_configs_ = sentinel_configs;
        redis_config_ = redis_config;

        if(sentinel_configs_.size()){
            SentinelOptions ops;
            ops.password = sentinel_configs_[0]->passwd;
            for (auto v : sentinel_configs_)
            {
                std::pair<std::string, int> node;
                node.first = v->host;
                node.second = v->port;
                ops.nodes.push_back(node);
            }
            ops.connect_timeout = std::chrono::milliseconds(redis_config_->connect_timeout);
            ops.socket_timeout = std::chrono::milliseconds(redis_config_->socket_timeout);
        
            sentinel_ = std::make_shared<Sentinel>(ops);
        }
        
        std::unique_lock<std::mutex> lock(redis_pool_mtx_);
        for (int i = 0; i < 16; i++)
        {
            std::shared_ptr<RedisPool> redisPool = std::make_shared<RedisPool>();
            redisPool->totalsize = 0;
            redisPool->usesize = 0;
            redisPool->idlesize = 0;
            redis_conn_pool_.push_back(redisPool);
        }
    }

    RedisPtr RedisMng::GetRedis2(bool master, int db)
    {
        int idx = (-1 == db) ? redis_config_->db : db;
        if (idx > 16 || idx < 0)
        {
            std::cout << "error redis idx:" << idx;
            return nullptr;
        }
        RedisPtr redis = nullptr;
        auto redisPool = GetRedisPool(idx);
        std::unique_lock<std::mutex> lock(redisPool->mtx);
        std::list<RealRedisPtr> &redis_queue = redisPool->redis_queue;
        for (auto iter = redis_queue.begin(); iter != redis_queue.end();)
        {

            if ((*iter) && (*iter)->redis != nullptr && (*iter)->master == master)
            {
                redisPool->usesize++;
                redisPool->idlesize--;
                redis = (*iter)->redis;
                redis_queue.erase(iter);
                return redis;
            }
            if (!(*iter) || (*iter)->redis == nullptr)
            {
                redisPool->totalsize--;
                iter = redis_queue.erase(iter);
            }
            else
            {
                iter++;
            }
        }
        if (redisPool->totalsize >= redis_config_->pool_size * 5)
        {

            std::cout << "no idle connection,idx:" << idx
                         << ",totalsize:" << redisPool->totalsize
                         << ",usesize:" << redisPool->usesize
                         << ",idlesize:" << redisPool->idlesize
                         << ",maxsize:" << redis_config_->pool_size * 5
                         << ",queue size:" << redis_queue.size();
            return nullptr;
        }

        ConnectionOptions connection_opts;
        connection_opts.host = redis_config_->host;                                                  // "172.24.13.83";  // Required.
        connection_opts.port = redis_config_->port;                                                  // 16379; // Optional. The default port is 6379.
        connection_opts.password = redis_config_->passwd;                                            // Optional. No password by default.
        connection_opts.connect_timeout = std::chrono::milliseconds(redis_config_->connect_timeout); // Required.
        connection_opts.socket_timeout = std::chrono::milliseconds(redis_config_->socket_timeout);   // Required.
        connection_opts.db = idx;
        connection_opts.keep_alive = true;

        ConnectionPoolOptions pool_opts;
        pool_opts.size = redis_config_->pool_size; // Optional. The default size is 1.

        redis = std::make_shared<Redis>(connection_opts, pool_opts);
        redisPool->totalsize++;
        redisPool->usesize++;
        redisPool->idlesize = 0;
        return redis;
    }

    RedisPtr RedisMng::GetRedis(bool master, int db)
    {
        if (!sentinel_)
        {
            return GetRedis2(master, db);
        }
        int idx = (-1 == db) ? redis_config_->db : db;
        if (idx > 16 || idx < 0)
        {
            std::cout << "error redis idx:" << idx;
            return nullptr;
        }
        RedisPtr redis = nullptr;
        auto redisPool = GetRedisPool(idx);
        std::unique_lock<std::mutex> lock(redisPool->mtx);
        std::list<RealRedisPtr> &redis_queue = redisPool->redis_queue;
        for (auto iter = redis_queue.begin(); iter != redis_queue.end();)
        {

            if ((*iter) && (*iter)->redis != nullptr && (*iter)->master == master)
            {
                redisPool->usesize++;
                redisPool->idlesize--;
                redis = (*iter)->redis;
                redis_queue.erase(iter);
                return redis;
            }
            if (!(*iter) || (*iter)->redis == nullptr)
            {
                redisPool->totalsize--;
                iter = redis_queue.erase(iter);
            }
            else
            {
                iter++;
            }
        }
        if (redisPool->totalsize >= redis_config_->pool_size * 5)
        {

            std::cout << "no idle connection,idx:" << idx
                         << ",totalsize:" << redisPool->totalsize
                         << ",usesize:" << redisPool->usesize
                         << ",idlesize:" << redisPool->idlesize
                         << ",maxsize:" << redis_config_->pool_size * 5
                         << ",queue size:" << redis_queue.size();
            return nullptr;
        }

        ConnectionOptions connection_opts;
        connection_opts.password = redis_config_->passwd;                                            // Optional. No password by default.
        connection_opts.connect_timeout = std::chrono::milliseconds(redis_config_->connect_timeout); // Required.
        connection_opts.socket_timeout = std::chrono::milliseconds(redis_config_->socket_timeout);   // Required.
        connection_opts.db = idx;

        ConnectionPoolOptions pool_opts;
        pool_opts.size = redis_config_->pool_size; // Optional. The default size is 1.

        redis = std::make_shared<Redis>(sentinel_, redis_config_->master_name, (master ? Role::MASTER : Role::SLAVE), connection_opts, pool_opts);
        redisPool->totalsize++;
        redisPool->usesize++;
        redisPool->idlesize = 0;
        return redis;
    }

    int RedisMng::GiveBack(RedisPtr redis, bool master, bool isvalid, int db)
    {
        int idx = (-1 == db) ? redis_config_->db : db;
        if (idx > 16 || idx < 0)
        {
            std::cout << "error redis idx:" << idx;
            return -1;
        }
        if (!isvalid)
        {
            // 对于非哨兵模式，直接使用原始的redis连接
            if (sentinel_)
            {
                ConnectionOptions connection_opts;
                connection_opts.password = redis_config_->passwd;                                            // Optional. No password by default.
                connection_opts.connect_timeout = std::chrono::milliseconds(redis_config_->connect_timeout); // Required.
                connection_opts.socket_timeout = std::chrono::milliseconds(redis_config_->socket_timeout);   // Required.
                connection_opts.db = idx;
                ConnectionPoolOptions pool_opts;
                pool_opts.size = redis_config_->pool_size; // Optional. The default size is 1.
                redis = std::make_shared<Redis>(sentinel_, redis_config_->master_name, Role::MASTER, connection_opts, pool_opts);
            }
            else
            {
                ConnectionOptions connection_opts;
                connection_opts.host = redis_config_->host;                                                  // "172.24.13.83";  // Required.
                connection_opts.port = redis_config_->port;                                                  // 16379; // Optional. The default port is 6379.
                connection_opts.password = redis_config_->passwd;                                            // Optional. No password by default.
                connection_opts.connect_timeout = std::chrono::milliseconds(redis_config_->connect_timeout); // Required.
                connection_opts.socket_timeout = std::chrono::milliseconds(redis_config_->socket_timeout);   // Required.
                connection_opts.db = idx;

                ConnectionPoolOptions pool_opts;
                pool_opts.size = redis_config_->pool_size; // Optional. The default size is 1.

                redis = std::make_shared<Redis>(connection_opts, pool_opts);
            }
        }
        auto redisPool = GetRedisPool(idx);
        std::unique_lock<std::mutex> lock(redisPool->mtx);
        auto real_redis = std::make_shared<RealRedis>();
        real_redis->master = master;
        real_redis->redis = redis;
        redisPool->redis_queue.push_back(real_redis);
        redisPool->idlesize++;
        redisPool->usesize--;
        return 0;
    }

    RedisPoolPtr RedisMng::GetRedisPool(int idx)
    {
        std::unique_lock<std::mutex> lock(redis_pool_mtx_);
        auto &redisPool = redis_conn_pool_[idx];
        if (!redisPool)
        {
            RedisPoolPtr tmpRedisPool = std::make_shared<RedisPool>();
            tmpRedisPool->idlesize = 0;
            tmpRedisPool->totalsize = 0;
            tmpRedisPool->usesize = 0;
            redis_conn_pool_[idx] = tmpRedisPool;
            redisPool = tmpRedisPool;
        }
        return redisPool;
    }

    RedisMngProxy::RedisMngProxy(bool master, int db)
    {
        master_ = master;
        db_ = db;
        isvalid_ = true;
        ptr_ = RedisMng::Instance()->GetRedis(master, db);
    }

    RedisMngProxy::~RedisMngProxy()
    {
        if (!ptr_)
        {
            isvalid_ = false;
        }
        RedisMng::Instance()->GiveBack(ptr_, master_, isvalid_, db_);
    }

    void RedisMngProxy::SetInvalid() { isvalid_ = false; }

    RedisPtr RedisMngProxy::operator->() const { return ptr_; }

    RedisMngProxy::operator void *() const { return ptr_.get(); }

    bool RedisMngProxy::operator!=(const RedisMngProxy &v) const
    {
        return ptr_ != v.ptr_;
    }

    bool RedisMngProxy::operator==(const RedisMngProxy &v) const
    {
        return ptr_ == v.ptr_;
    }

    bool RedisMngProxy::operator==(nullptr_t) const { return !ptr_; }

    bool RedisMngProxy::operator!=(nullptr_t) const
    {
        return (bool)ptr_;
    }
} // namespace lhserver
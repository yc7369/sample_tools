/*
 * @FilePath: /lh_framework/include/database/dbman.h
 * @Brief: 数据库连接池管理类
 * @Version: 1.0
 * @Date: 2020-06-06 10:34:50
 * @Author: yangchen
 * @Copyright: Copyright (c) 2021 LeHighHongKing All rights reserved.
 * @LastEditors: yangchen
 * @LastEditTime: 2021-10-13 16:49:07
 */

#ifndef LHSERVER_DBMAN_H
#define LHSERVER_DBMAN_H

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "basedb.h"
#include "utils/singleton.h"

namespace sql {

using BaseDBPtr = std::shared_ptr<BaseDB>;
using BaseDBPool = std::deque<BaseDBPtr>;
using DBPools = std::deque<BaseDBPool>;

typedef struct {
    std::string driverName;
    std::string addr;
    std::string user;
    std::string passwd;
    std::string db;
    int pool_size;
} DBConnConfig;

class DBMan : public Singleton<DBMan> {
public:
    DBMan();
    virtual ~DBMan();

    /**
     * @brief: 通过配置文件初始化数据库连接池
     * @author: yangchen
     * @param[in] config : 数据库配置列表
     * @param[out]
     * @return true成功或false失败
     */
    bool Init(std::vector<DBConnConfig> config);

    /**
     * @brief: 释放连接池
     * @author: yangchen
     * @param[in]
     * @param[out]
     * @return
     */
    void Release();

    /**
     * @brief: 从连接池中获取一个BaseDBPtr对象
     * @author: yangchen
     * @param[in]
     * @param[out]
     * @return 空闲的数据库连接对象
     */
    BaseDBPtr GetDB();

    /**
     * @brief: 返回BaseDBPtr对象给连接池
     * @author: yangchen
     * @param[in] 数据库连接对象
     * @param[out]
     * @return
     */
    void PushDB(BaseDBPtr db);

private:
    /**
     * @brief: 切换数据库（主库与备库切换）
     * @author: yangchen
     * @param[in]
     * @param[out]
     * @return
     */
    void SwapDBPool();

    /**
     * @brief: 初始化数据库
     * @author: yangchen
     * @param[in] driverName 数据库驱动
     * @param[in] addr 地址
     * @param[in] user 用户名
     * @param[in] passwd 密码
     * @param[in] db 数据库名
     * @param[in] pool 数据量连接池
     * @param[in] poolSize 数据库连接池大小
     * @param[out]
     * @return 0成功，非0失败
     */
    int Init(const std::string& driverName,
             const std::string& addr,
             const std::string& user,
             const std::string& passwd,
             const std::string& db,
             BaseDBPool& pool,
             int poolSize = 10);

    /**
     * @brief: 初始化数据库
     * @author: yangchen
     * @param[in] driverName 数据库驱动
     * @param[in] addr 地址
     * @param[in] user 用户名
     * @param[in] passwd 密码
     * @param[in] db 数据库名
     * @param[in] poolSize 数据库连接池大小
     * @param[out]
     * @return true成功，false失败
     */
    bool Init(const std::string& driverName,
              const std::string& addr,
              const std::string& user,
              const std::string& passwd,
              const std::string& db,
              int poolSize = 10);

private:
    std::mutex mutex_;
    size_t nextIndex_;
    int poolSize_;
    std::shared_ptr<std::thread> check_thread_;
    BaseDBPool dbPool_;    // 当前可用线程池
    DBPools backup_pool_;  // 备份线程池
    bool check_running_ = false;

    std::shared_ptr<std::thread> active_thread_;  // 连接保活线程
    bool active_running_ = false;
};

/// 帮助类,自动获取和释放数据库对象
class DBHolder {
public:
    DBHolder() {
        db_ = DBMan::Instance()->GetDB();
    }

    ~DBHolder() {
        DBMan::Instance()->PushDB(db_);
    }

    BaseDBPtr GetDB() {
        return db_;
    }

private:
    BaseDBPtr db_;
};

}  // namespace sql
#endif  // LHSERVER_DBMAN_H

//
// Created by yangchen on 6/8/20.

// Copyright (c) 2020 LeHighHongKing All rights reserved.
//
#include "database/dbman.h"

#include <sstream>

#include "database/mysqldb.h"
#include "zlog/ztp_log.h"

namespace sql {

void ActivePool(BaseDBPool& pool, size_t size);
#define MAX_POOL_SIZE 100

DBMan::DBMan() {
    nextIndex_ = 0;
    poolSize_ = 0;
}

DBMan::~DBMan() {
    Release();
}

bool DBMan::Init(const std::string& driverName,
                 const std::string& addr,
                 const std::string& user,
                 const std::string& passwd,
                 const std::string& db,
                 int poolSize) {
    return Init(driverName, addr, user, passwd, db, dbPool_, poolSize) != 0;
}

void DBMan::Release() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto db : dbPool_) {
        db.reset();
    }

    dbPool_.clear();
    backup_pool_.clear();
    check_running_ = false;
    active_running_ = false;
}

BaseDBPtr DBMan::GetDB() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (dbPool_.size() == 0) {
        ZLOG_ERROR("GetDB() : no database avalible in pool, pool size is zero");

        return nullptr;
    }

    //    if (nextIndex_ >= (dbPool_.size()-1)) {   // 最后一个用于检测线程
    //        nextIndex_ = 0;
    //    }
    //
    //    return dbPool_[nextIndex_++];
    // 以上代码注释掉是为了避免db对象被用于不同线程中

    if (dbPool_.size() >= 2) {  // 最后一个用于检测线程,所以要>=2
        auto db = dbPool_.front();
        dbPool_.pop_front();
        return db;
    }

    ZLOG_ERROR("GetDB() : no database avalible in pool");
    return nullptr;
}

void DBMan::PushDB(BaseDBPtr db) {
    if (!db) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t sn = db->GetDbSn();
    if (dbPool_.front()->GetDbSn() == sn) {
        size_t size = dbPool_.size();
        dbPool_.insert(dbPool_.begin() + size - 1, 1,
                       db);  // 插入到倒数第二位置
        //    dbPool_.push_front(db); // 最后一个用于检测线程,所以这里放入头部
    } else {
        for (auto iter : backup_pool_) {
            if (iter.front()->GetDbSn() == sn) {
                size_t size = iter.size();
                iter.insert(iter.begin() + size - 1, 1,
                            db);  // 插入到倒数第二位置
                break;
            }
        }
    }
}

int DBMan::Init(const std::string& driverName,
                const std::string& addr,
                const std::string& user,
                const std::string& passwd,
                const std::string& db,
                sql::BaseDBPool& pool,
                int poolSize) {
    if (driverName.empty() || addr.empty() || user.empty() || passwd.empty()) {
        return 0;
    }

    if (poolSize <= 0 || poolSize > MAX_POOL_SIZE) {
        poolSize = MAX_POOL_SIZE;
    }

    int count = 0;
    if (driverName.compare("mysql") == 0) {
        for (int i = 0; i < poolSize; i++) {
            auto database = std::make_shared<MysqlDB>();
            auto succ = database->Open(addr, user, passwd, db);
            if (succ) {
                pool.emplace_back(database);
                count++;
            } else {
                std::ostringstream ostrm;
                ostrm << "cannot open database connetion, addr : " << addr << ", user : " << user;
                std::cout << ostrm.str() << std::endl;
                ZLOG_ERROR("{}", ostrm.str());
            }
        }
    } else {

        std::ostringstream ostrm;
        ostrm << "unsupport database driver";
        std::cout << ostrm.str() << std::endl;
        ZLOG_ERROR("{}", ostrm.str());

        return 0;
    }

    if (count != poolSize) {
        std::ostringstream ostrm;
        ostrm << "warnning!!Not all database connection made.[" << count << "-->" << poolSize
              << "]";
        std::cout << ostrm.str() << std::endl;
        ZLOG_WARN("{}", ostrm.str());
    }
    return count;
}

bool DBMan::Init(std::vector<sql::DBConnConfig> config) {
    if (config.empty()) {
        return false;
    }

    int index = 0;
    for (auto c : config) {
        BaseDBPool pool;
        int cnt = Init(c.driverName,
                       c.addr,
                       c.user,
                       c.passwd,
                       c.db,
                       pool,
                       c.pool_size + 1);  // 这里size加1是为了有个连接用来检测数据库的可用性
        if (cnt > 0) {
            if (0 == index) {
                dbPool_ = std::move(pool);  // 第一个作为主连接池
            } else {
                backup_pool_.emplace_back(pool);  // 其他作为备用连接池
            }
            index++;
        } else {
            ZLOG_WARN("not all database init success");
        }
    }

    if (backup_pool_.empty()) {
        ZLOG_WARN("no backup database");
    }

    // 启动监听线程检查连接是否有效
    check_running_ = true;
    check_thread_ = std::make_shared<std::thread>([&, this]() {
        if (dbPool_.empty()) {
            ZLOG_ERROR("no databasee avalible");

            return;
        }
        auto db = dbPool_.back();
        while (check_running_) {
            if (!db->IsValid()) {
                ZLOG_WARN("database is invalid");
                // 进行重连
                bool reconnect = false;
                for (int i = 0; i < 5; i++) {  // 尝试重连5次
                    if (db->Reconnect()) {
                        reconnect = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }

                if (reconnect) {
                    ZLOG_INFO("db monitor reconnected success");
                    continue;
                }

                if (backup_pool_.empty()) {
                    ZLOG_ERROR("no backup databases");
                    return;
                }

                // 重连不成功，切换到备库
                SwapDBPool();
                db = dbPool_.back();
            }

            std::string sql = "SELECT 1=1 AS RESULT";
            SQLError err = SQLError::SUCCESS;
            auto res = db->Query(sql, err);
            if (!res || err != SQLError::SUCCESS) {
                ZLOG_WARN("check database failed");
                if (backup_pool_.empty()) {
                    ZLOG_ERROR("no backup databases");
                    return;
                }

                SwapDBPool();
                db = dbPool_.back();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // 启动保活线程,定时每个数据库连接操作一次
    active_running_ = true;
    active_thread_ = std::make_shared<std::thread>([&, this]() {
        if (dbPool_.empty()) {
            active_running_ = false;
            ZLOG_WARN("no database object to active");

            return;
        }
        while (active_running_) {
            {

                ZLOG_WARN("active main database");
                std::lock_guard<std::mutex> lock(mutex_);
                ActivePool(dbPool_,
                           dbPool_.size() - 1);  // 当前连接池的最后一个连接是用来检测的
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (size_t i = 0; i < backup_pool_.size(); i++) {
                    std::ostringstream ostrm;
                    ostrm << "active backup database[" << i << "]";
                    ZLOG_WARN("{}", ostrm.str());
                    ActivePool(backup_pool_[i], backup_pool_[i].size());
                }
            }
            std::this_thread::sleep_for(std::chrono::minutes(57));
        }
    });

    check_thread_->detach();
    active_thread_->detach();

    return true;
}

/// 交换连接池
void DBMan::SwapDBPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (backup_pool_.empty()) {
        ZLOG_ERROR("no backup databases");

        return;
    }

    ZLOG_INFO("swap database pool...");

    backup_pool_.push_back(dbPool_);
    dbPool_ = backup_pool_.front();
    backup_pool_.pop_front();
}

/// size个连接进行保活,即执行一条sql
void ActivePool(BaseDBPool& pool, size_t size) {
    for (size_t i = 0; i < size; i++) {
        std::string sql = "SELECT 1=1 AS RESULT;";
        SQLError err = SQLError::SUCCESS;
        auto res = pool[i]->Query(sql, err);
        if (!res || err != SQLError::SUCCESS) {
            ZLOG_WARN("active database failed");

            // 进行重连
            bool reconnect = false;
            for (int m = 0; m < 5; m++) {  // 尝试重连5次
                if (pool[i]->Reconnect()) {
                    reconnect = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }

            if (reconnect) {
                ZLOG_INFO("database reconnected success");
                continue;
            }
        }
    }
}

}  // namespace sql

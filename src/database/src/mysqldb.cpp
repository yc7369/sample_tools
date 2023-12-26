//
// Created by yangchen on 6/6/20.

// Copyright (c) 2023  All rights reserved.
//

#include "database/mysqldb.h"

#include <sstream>

#include "zlog/ztp_log.h"

namespace sql {

class DriverHolder {
public:
    DriverHolder() {
        driver_ = mysql::get_mysql_driver_instance();
        driver_->threadInit();
    }

    virtual ~DriverHolder() {
        if (driver_) {
            driver_->threadEnd();
        }
    }

    mysql::MySQL_Driver* GetDriver() {
        return driver_;
    }

private:
    mysql::MySQL_Driver* driver_ = nullptr;
};

// DriverHolder driver_holder;

MysqlDB::MysqlDB() {
    open_ = false;
    point_.reset(nullptr);
}

MysqlDB::~MysqlDB() {
    Close();
}

bool MysqlDB::Open(const std::string& addr,
                   const std::string& user,
                   const std::string& passwd,
                   const std::string& db) {
    if (addr.empty() || user.empty() || passwd.empty()) {
        ZLOG_ERROR("empty parameters!! ");
        return false;
    }

    addr_ = addr;
    user_ = user;
    passwd_ = passwd;
    db_ = db;

    size_t h1 = std::hash<std::string>()(addr_);
    size_t h2 = std::hash<std::string>()(db_);
    db_sn_ = h1 ^ (h2 << 1);

    auto driver = mysql::get_mysql_driver_instance();
    if (nullptr == driver) {
        ZLOG_ERROR("cannot get mysql driver instance ");
        return false;
    }

    try {
        std::string conn_str = addr_ + "/";
        conn_str += db;
        //        driver->threadInit();   // 初始化线程环境
        auto conn = driver->connect(conn_str.c_str(), user_, passwd_);
        if (!conn) {
            ZLOG_ERROR(
                "connect  mysql failed,conn_str: {} ,user: {} ,passwd:", conn_str, user_, passwd_);

            return false;
        } else {
            connection_.reset(conn);
            if (connection_) {
                open_ = true;
            }
        }
    } catch (sql::SQLException& e) {
        ZLOG_ERROR("Connected to  database failed :sql errcode:{},error_msg:{}",
                   e.getErrorCode(),
                   e.what());
        return false;
    } catch (std::exception& e) {
        ZLOG_ERROR("Connected to  database failed error_msg: {}", e.what());
        return false;
    } catch (...) {
        ZLOG_ERROR("unknown exception");
        return false;
    }

    // TODO 如何切换db
    return true;
}

void MysqlDB::Close() {
    if (!open_ || !connection_) {
        open_ = false;
        return;
    }

    connection_->close();
    open_ = false;
    connection_.reset();
}

int MysqlDB::Exec(const std::string& sql, SQLError& ret_code) {
    int ret = 0;
    ret = MysqlExec(sql, ret_code);
    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        return ret;
    }

    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {
            if (is_begin_trans_) {
                ZLOG_ERROR("reconnect failed");
                return -1;
            }
            ret = MysqlExec(sql, ret_code);
            return ret;
        }
    }

    ZLOG_ERROR("reconnncet 3 times and failed");
    return -1;
}

std::unique_ptr<PreparedStatement> MysqlDB::Prepared(const std::string& sql, SQLError& ret_code) {
    std::unique_ptr<PreparedStatement> stmt_ptr = MysqlPrepared(sql, ret_code);
    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        return stmt_ptr;
    }

    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {
            if (is_begin_trans_) {
                ZLOG_ERROR("reconect failed.");
                return nullptr;
            }
            return MysqlPrepared(sql, ret_code);
        }
    }

    ZLOG_ERROR("reconnncet 3 times and failed");
    return nullptr;
}

std::unique_ptr<ResultSet> MysqlDB::Query(const std::string& sql, SQLError& ret_code) {
    std::unique_ptr<ResultSet> rst_ptr = MysqlQuery(sql, ret_code);

    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        return rst_ptr;
    }

    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {
            if (is_begin_trans_) {
                ZLOG_ERROR("reconnect failed");
                return nullptr;
            }
            return MysqlQuery(sql, ret_code);
        }
    }

    ZLOG_ERROR("reconnect 3 times and failed");
    return nullptr;
}

uint64_t MysqlDB::GetDbSn() {
    return db_sn_;
}

bool MysqlDB::IsOpen() {
    return open_ && connection_;
}

bool MysqlDB::Reconnect() {
    if (connection_) {
        if (connection_->isValid()) {
            connection_->close();
        }
    }

    return Open(addr_, user_, passwd_, db_);
}

bool MysqlDB::IsValid() {
    if (!open_ || !connection_) {
        return false;
    }

    try {
        return connection_->isValid();
    } catch (sql::SQLException& e) {

        ZLOG_ERROR("check mysql valid failed, ,sql errcode: {} ,error_msg: {}",
                   e.getErrorCode(),
                   e.what());
    } catch (std::exception& e) {

        ZLOG_ERROR("check mysql valid failed,error_msg: {}", e.what());

    } catch (...) {
        ZLOG_ERROR("unknown exception");
    }

    return false;
}

int MysqlDB::AutoCommit(const bool bflag) {
    is_begin_trans_ = !bflag;
    SQLError ret_code;
    int ret = MysqlAutoCommit(bflag, ret_code);
    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        if (ret != 0)
            is_begin_trans_ = false;
        return ret;
    }

    is_begin_trans_ = false;
    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {
            ZLOG_ERROR("reconnncet failed");
            return -1;
        }
    }

    ZLOG_ERROR("reconnncet 3 times and failed");
    return -1;
}

int MysqlDB::BeginTrans() {
    idx_ = 0;
    return 0;
}

int MysqlDB::SetSavePoint() {
    return MysqlSetSavePoint();
}

int MysqlDB::Commit() {
    SQLError ret_code;
    int ret = MysqlCommit(ret_code);

    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        return ret;
    }

    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {

            ZLOG_ERROR("reconnncet failed");
            return -1;
        }
    }

    ZLOG_ERROR("reconnncet 3 times and failed");
    return -1;
}

int MysqlDB::Rollback(bool flag) {
    SQLError ret_code;
    int ret = MysqlRollback(flag, ret_code);
    if (ret_code != SQLError::MYSQL_SERVER_GONE_AWAY) {
        return ret;
    }

    int count = 3;
    while (count-- > 0) {
        if (Reconnect()) {
            ZLOG_ERROR("reconnncet failed");
            return -1;
        }
    }

    ZLOG_ERROR("reconnncet 3 times and failed");
    return -1;
}

int MysqlDB::MysqlExec(const std::string& sql, SQLError& ret_code) {
    if (!open_ || !connection_) {
        ZLOG_ERROR("not connected");
        ret_code = SQLError::EMPTY_CONNECTION;
        return -1;
    }
    if (sql.empty()) {
        ZLOG_ERROR("sql empty");
        ret_code = SQLError::EMPTY_SQL;
        return -1;
    }

    try {
        ret_code = SQLError::SUCCESS;
        Statement* stmt = connection_->createStatement();
        if (!stmt) {
            ret_code = SQLError::STATMENT_NULL;
            ZLOG_ERROR("createStatement failed");
            return -1;
        }
        stmt->execute(sql);
        return stmt->getUpdateCount();
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();

        ZLOG_ERROR(
            "Query sql :{} failed, sql errcode: {},error_msg:{}", sql, e.getErrorCode(), e.what());
        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
        return -1;
    } catch (std::exception& e) {
        ZLOG_ERROR("Query sql :{} failed,  {},error_msg:{}", sql, e.what());
        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    } catch (...) {

        ZLOG_ERROR("unknown exception");

        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    }

    ret_code = SQLError::SUCCESS;
    return 0;
}

std::unique_ptr<PreparedStatement> MysqlDB::MysqlPrepared(const std::string& sql,
                                                          SQLError& ret_code) {
    if (!open_ || !connection_) {
        ret_code = SQLError::NOT_CONNECTED;
        ZLOG_ERROR("not connected");

        return nullptr;
    }
    if (sql.empty()) {
        ret_code = SQLError::EMPTY_SQL;
        ZLOG_ERROR("sql empty");
        return nullptr;
    }

    try {
        ret_code = SQLError::SUCCESS;
        return std::unique_ptr<PreparedStatement>(connection_->prepareStatement(sql.c_str()));
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();
        ZLOG_ERROR("Prepared sql :{} failed, sql errcode: {},error_msg:{}",
                   sql,
                   e.getErrorCode(),
                   e.what());
        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
    } catch (std::exception& e) {
        ZLOG_ERROR("Prepared sql :{} failed, error_msg:{}", sql, e.what());
        ret_code = SQLError::SQL_EXCEPTION;
    } catch (...) {
        ZLOG_ERROR("unknown exception");
        ret_code = SQLError::SQL_EXCEPTION;
    }

    return nullptr;
}

std::unique_ptr<ResultSet> MysqlDB::MysqlQuery(const std::string& sql, SQLError& ret_code) {
    if (!open_ || !connection_) {
        ret_code = SQLError::NOT_CONNECTED;
        return nullptr;
    }
    if (sql.empty()) {
        ret_code = SQLError::EMPTY_SQL;
        return nullptr;
    }

    auto stmt = MysqlPrepared(sql, ret_code);
    if (!stmt) {
        if (ret_code == SQLError::SUCCESS) {
            ret_code = SQLError::STATMENT_NULL;
        }
        ZLOG_ERROR("createStatement failed");
        return nullptr;
    }

    try {
        ret_code = SQLError::SUCCESS;
        return std::unique_ptr<ResultSet>(stmt->executeQuery());
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();
        ZLOG_ERROR(
            "Query sql :{} failed, sql errcode: {},error_msg:{}", sql, e.getErrorCode(), e.what());

        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
    } catch (std::exception& e) {
        ZLOG_ERROR("Query sql :{} failed, error_msg:{}", sql, e.what());

        ret_code = SQLError::SQL_EXCEPTION;
    } catch (...) {
        ZLOG_ERROR("unknown exception");

        ret_code = SQLError::SQL_EXCEPTION;
    }

    return nullptr;
}

int MysqlDB::MysqlAutoCommit(const bool bflag, SQLError& ret_code) {
    if (!open_ || !connection_) {

        ZLOG_ERROR("connection is null");
        ret_code = SQLError::EMPTY_CONNECTION;
        return -1;
    }

    try {
        connection_->setAutoCommit(bflag);
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();
        ZLOG_ERROR("mysql set autocommit failed, sql errcode: {},error_msg:{}",
                   e.getErrorCode(),
                   e.what());
        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
        return -1;
    } catch (std::exception& e) {
        ZLOG_ERROR("mysql set autocommit failed, error_msg:{}", e.what());
        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    } catch (...) {
        ZLOG_ERROR("unknown exception");
        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    }
    return 0;
}

int MysqlDB::MysqlSetSavePoint() {
    if (!open_ || !connection_) {

        ZLOG_ERROR("connection is null");

        return -1;
    }

    try {
        std::ostringstream point;
        point << "POINT_" << idx_++;
        point_.reset(connection_->setSavepoint(point.str()));
    } catch (sql::SQLException& e) {
        ZLOG_ERROR("SetSavePoint failed, sql errcode: {},error_msg:{}", e.getErrorCode(), e.what());

        return e.getErrorCode();
    } catch (std::exception& e) {
        ZLOG_ERROR("SetSavePoint failed, error_msg:{}", e.what());
        return -1;
    } catch (...) {

        ZLOG_ERROR("unknown exception");
        return -1;
    }
    return 0;
}

int MysqlDB::MysqlCommit(SQLError& ret_code) {
    if (!open_ || !connection_) {
        ret_code = SQLError::EMPTY_CONNECTION;
        ZLOG_ERROR("connection is null");

        return -1;
    }

    try {
        connection_->commit();
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();
        ZLOG_ERROR(
            "mysql commit  failed, sql errcode: {},error_msg:{}", e.getErrorCode(), e.what());

        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
        return -1;
    } catch (std::exception& e) {
        ZLOG_ERROR("mysql commit  failed, error_msg:{}", e.what());

        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    } catch (...) {

        ZLOG_ERROR("unknown exception");
        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    }
    return 0;
}

int MysqlDB::MysqlRollback(bool flag, SQLError& ret_code) {
    if (!open_ || !connection_) {
        ret_code = SQLError::EMPTY_CONNECTION;
        ZLOG_ERROR("connection is null");

        return -2;
    }

    try {
        if (flag) {
            connection_->rollback(point_.get());
            connection_->releaseSavepoint(point_.get());
        } else {
            connection_->rollback();
        }
    } catch (sql::SQLException& e) {
        ret_code = SQLError::MYSQL_SERVER_GONE_AWAY;
        int errcode = e.getErrorCode();
        ZLOG_ERROR(
            "mysql rollback  failed, sql errcode: {},error_msg:{}", e.getErrorCode(), e.what());
        if (errcode != 2006 && errcode != 2013) {
            ret_code = SQLError::SQL_EXCEPTION;
        }
        return -1;
    } catch (std::exception& e) {
        ZLOG_ERROR("mysql rollback  failed, error_msg:{}", e.what());

        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    } catch (...) {
        ZLOG_ERROR("unknown exception");
        ret_code = SQLError::SQL_EXCEPTION;
        return -1;
    }
    return 0;
}

}  // namespace sql
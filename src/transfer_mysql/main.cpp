#include "concurrentqueue.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/metadata.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/statement.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include "slog.h"
#include "util.h"
#include <chrono>
#include <thread>

#define MYSQL_SECTION "mysql"

struct MysqlInfo {
    std::string url_;        // 数据库ip地址
    std::string user_;       // 数据库用户名
    std::string pass_;       // 数据库密码
    std::string data_base_;  // 数据库名字
};

class DBWorkHandle {
public:
    ~DBWorkHandle() {
        exit_ = true;
        if (check_thread_.joinable()) {
            check_thread_.join();
        }

        DisConnect(true);
    }

    void DisConnect(bool destroy = false) {
        if (stat_) {
            stat_->close();
            delete stat_;
            stat_ = nullptr;
        }

        if (conn_) {
            conn_->close();
            delete conn_;
            conn_ = nullptr;
        }

        if (driver_ && destroy) {
            driver_->threadEnd();
            delete driver_;
            driver_ = nullptr;
        }
    }

    bool Connect() {
        try {
            conn_ = driver_->connect(
                dbinfo_.url_.c_str(), dbinfo_.user_.c_str(), dbinfo_.pass_.c_str());
            if (conn_ == NULL) {
                BSLOG_ERROR("UKOpen conn is null");
                return false;
            }
            conn_->setSchema(dbinfo_.data_base_.c_str());

            auto stmt = conn_->createStatement();
            if (stmt == NULL) {
                BSLOG_ERROR("stmt is null");
                return false;
            }
        } catch (sql::SQLException& e) {
            BSLOG_ERROR("Connected to  database failed, sql errcode:{},error_msg:{}",
                        e.getErrorCode(),
                        e.what());
            return false;
        } catch (std::exception& e) {
            BSLOG_ERROR("Connected to  database failed, error_msg:{}", e.what());
            return false;
        }

        return true;
    }

    bool Init(const MysqlInfo& dbinfo) {
        dbinfo_ = dbinfo;
        try {
            driver_ = sql::mysql::get_mysql_driver_instance();
            if (driver_ == NULL) {
                return false;
            }
            if (!Connect()) {
                return false;
            }
            check_thread_ = std::thread(&DBWorkHandle::CheckStatus, this);
            update_thread_ = std::thread(&DBWorkHandle::UpdateThread, this);
        } catch (std::exception& e) {
            DisConnect();
            BSLOG_ERROR("DBconf Conect Err: {}", e.what());
        }

        return true;
    }

    void CheckStatus() {
        while (!exit_) {
            static int count;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> l(mu);
            try {
                if (conn_ && conn_->isValid()) {
                    if (++count > 1000) {
                        count = 0;
                        auto stmt = conn_->prepareStatement("select 1 = 1;");
                        if (stmt) {
                            stmt->executeQuery();
                        }
                    }
                } else {
                    DisConnect();
                    Connect();
                }
            } catch (std::exception& e) {
                BSLOG_ERROR("check status Err: {}", e.what());
            }
        }
    }

    void UpdateThread() {
        std::string sql;
        while (!exit_) {
            while (q.try_dequeue(sql)) {
                std::lock_guard<std::mutex> l(mu);
                try {
                    if (!conn_ || !conn_->isValid()) {
                        sleep(1);
                        continue;
                    }

                    auto stmt = conn_->prepareStatement(sql);
                    if (stmt) {
                        stmt->executeUpdate();
                    }
                } catch (std::exception& e) {
                    BSLOG_ERROR("onmsg Err: error:{} sql: {}", e.what(), sql);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void OnMsg(const std::string& sql) {
        q.enqueue(sql);
    }

private:
    MysqlInfo dbinfo_;
    bool exit_ = false;
    std::thread check_thread_;
    std::thread update_thread_;
    sql::Statement* stat_;
    sql::Connection* conn_;
    sql::mysql::MySQL_Driver* driver_;

    std::mutex mu;

    moodycamel::ConcurrentQueue<std::string> q;
};

int main(int argc, char** argv) {

    MysqlInfo dbinfo;

    // dbinfo.url_ = ini.Get(MYSQL_SECTION, "host", "tcp://172.24.13.178:3306");
    // dbinfo.user_ = ini.Get(MYSQL_SECTION, "account", "chronos");
    // dbinfo.pass_ = ini.Get(MYSQL_SECTION, "password", "chronos");
    // dbinfo.data_base_ = ini.Get(MYSQL_SECTION, "database", "yctest");
    // SlogInit(opt.log_dir, opt.log_level);

    // BSLOG_INFO("server start!");

    // mybase::TimerMan::instance()->run();
    // auto dbw = std::make_shared<DBWorkHandle>();
    // if (!dbw->Init(dbinfo)) {
    //     return -1;
    // }

    return 0;
}
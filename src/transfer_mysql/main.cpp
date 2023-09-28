#include "concurrentqueue.h"
#include "config_api.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/metadata.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/statement.h"
#include "json.hpp"
#include "mybase/timerman.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include "slog.h"
#include "util.h"
#include <chrono>
#include <fstream>
#include <thread>

#define MYSQL_SECTION "mysql"

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

bool ReadConfig(const std::string& file, ConfigOpt opts) {
    using namespace nlohmann;
    json cfg;
    try {
        std::ifstream ifs(file);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        cfg = json::parse(content);
    } catch (const std::exception& e) {
        std::cout << "parse config failed, {}" << e.what();
        exit(-1);
    }
    std::cout << "config file parse success. cfg:{}" << cfg.dump();

    if (cfg.contains(MYSQL_SECTION)) {
        json mysql = cfg[MYSQL_SECTION];
        opts.dbinfo_.url_ = mysql["host"];
        opts.dbinfo_.user_ = mysql["account"];
        opts.dbinfo_.pass_ = mysql["password"];
        opts.dbinfo_.data_base_ = mysql["database"];
    }
    if (cfg.contains("base")) {
        json base = cfg["base"];
        // 日志配置
        opts.base_.log_dir = base["log_dir"];
        opts.base_.log_level = base["log_level"];
    }

    return true;
}

int main(int argc, char** argv) {

    std::string filename = "config.json";
    ConfigOpt opts;
    ReadConfig(filename, opts);

    SlogInit(opts.base_.log_dir, opts.base_.log_level);

    BSLOG_INFO("server start!");

    mybase::TimerMan::instance()->run();
    auto dbw = std::make_shared<DBWorkHandle>();
    if (!dbw->Init(opts.dbinfo_)) {
        return -1;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
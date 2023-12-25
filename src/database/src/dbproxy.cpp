#include "database/dbproxy.h"

#include <future>
#include <thread>

#include "string_assist.h"
#include "utils/lh_exception.h"
namespace sql {

DBProxy::~DBProxy() {}

int DBProxy::Exec(const std::string& sql, SQLError& ret_code, uint16_t index) {
    try {
        if (!CheckSql(const_cast<std::string&>(sql), SQL_OPER_TYPE::SELECT)) {
            ret_code = SQLError::SQL_ERROR;
            return -1;
        }
        std::future<DBProxyResult<int>> fut = std::async(std::launch::async, [&]() {
            DBProxyResult<int> rst;
            BaseDBPtr ptr = GetDB();
            if (ptr == nullptr) {
                rst.err_code = SQLError::EMPTY_CONNECTION;
                rst.result = -1;
                return rst;
            }

            rst.err_code = SQLError::SUCCESS;
            rst.result = ptr->Exec(sql, rst.err_code);

            DBMan::Instance()->PushDB(ptr);
            return rst;
        });

        if (fut.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) {
            ret_code = SQLError::TIMEOUT;
            return -1;
        }

        DBProxyResult<int> rst = std::move(fut.get());

        ret_code = rst.err_code;
        return rst.result;
    } catch (std::exception& e) {
        std::string excep_info = "exec exception,what():";
        excep_info.append(e.what());
        throw lhserver::lh_exception(excep_info);
    } catch (...) {
        std::string excep_info = "exec exception,what():";
        excep_info.append("unkonwn exception");
        throw lhserver::lh_exception(excep_info);
    }
}

std::unique_ptr<PreparedStatement>
DBProxy::Prepared(const std::string& sql, SQLError& ret_code, uint16_t index) {
    try {
        std::future<DBProxyResult<std::unique_ptr<PreparedStatement>>> fut =
            std::async(std::launch::async, [&]() {
                DBProxyResult<std::unique_ptr<PreparedStatement>> rst;
                BaseDBPtr ptr = GetDB();
                if (ptr == nullptr) {
                    rst.err_code = SQLError::EMPTY_CONNECTION;
                    rst.result = nullptr;
                    return rst;
                }

                rst.err_code = SQLError::SUCCESS;
                rst.result = ptr->Prepared(sql, rst.err_code);

                DBMan::Instance()->PushDB(ptr);
                return rst;
            });

        if (fut.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) {
            ret_code = SQLError::TIMEOUT;
            return nullptr;
        }

        DBProxyResult<std::unique_ptr<PreparedStatement>> rst = std::move(fut.get());

        ret_code = rst.err_code;
        return std::move(rst.result);
    } catch (std::exception& e) {
        std::string excep_info = "prepared exception,what():";
        excep_info.append(e.what());
        throw lhserver::lh_exception(excep_info);
    } catch (...) {
        std::string excep_info = "prepared exception,what():";
        excep_info.append("unkonwn exception");
        throw lhserver::lh_exception(excep_info);
    }
}

std::unique_ptr<ResultSet>
DBProxy::Query(const std::string& sql, SQLError& ret_code, uint16_t index) {
    try {
        std::future<DBProxyResult<std::unique_ptr<ResultSet>>> fut =
            std::async(std::launch::async, [&]() {
                DBProxyResult<std::unique_ptr<ResultSet>> rst;
                BaseDBPtr ptr = GetDB();
                if (ptr == nullptr) {
                    rst.err_code = SQLError::EMPTY_CONNECTION;
                    rst.result = nullptr;
                    return rst;
                }

                rst.err_code = SQLError::SUCCESS;
                rst.result = ptr->Query(sql, rst.err_code);

                DBMan::Instance()->PushDB(ptr);
                return rst;
            });

        if (fut.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) {
            ret_code = SQLError::TIMEOUT;
            return nullptr;
        }

        DBProxyResult<std::unique_ptr<ResultSet>> rst = std::move(fut.get());
        ret_code = rst.err_code;
        return std::move(rst.result);
    } catch (std::exception& e) {
        std::string excep_info = "query exception,what():";
        excep_info.append(e.what());
        throw lhserver::lh_exception(excep_info);
    } catch (...) {
        std::string excep_info = "query exception,what():";
        excep_info.append("unkonwn exception");
        throw lhserver::lh_exception(excep_info);
    }
}

BaseDBPtr DBProxy::GetDB(int times) {
    if (times <= 0) {
        return nullptr;
    }
    BaseDBPtr ptr = DBMan::Instance()->GetDB();
    if (ptr != nullptr) {
        return ptr;
    }

    return GetDB(times--);
}

bool DBProxy::CheckSql(std::string& strsql, const SQL_OPER_TYPE& type) {
    std::string sqlflag = get_sql_flag(type);
    std::string sqls = util::StringAssist::ltrim(strsql);
    std::string flag = util::StringAssist::rtrim(util::StringAssist::ltrim(sqlflag));
    for (size_t idx = 0; idx < flag.length(); idx++) {
        if (tolower(flag[idx]) != tolower(strsql[idx])) {
            return true;
        }
    }
    return false;
}

}  // namespace sql

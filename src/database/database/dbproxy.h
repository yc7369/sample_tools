/*
 * @FilePath: dbproxy.h
 * @Brief: 数据库接口封装类
 * @Version: 1.0
 * @Date: 2023-10-26 19:31:16
 * @Author: yangchen
 * @Copyright: Copyright (c) 2021  All rights reserved.
 * @LastEditors: yangchen
 * @LastEditTime: 2021-10-11 11:27:37
 */

#ifndef __LHSERVER_DB_PROXY_H__
#define __LHSERVER_DB_PROXY_H__

#include <string>

#include "basedb.h"
#include "dbman.h"

namespace sql {

// 数据库操作结果
template <typename T>
struct DBProxyResult {
    SQLError err_code;  // 错误码
    T result;           // 返回的结果
};

enum SQL_OPER_TYPE {
    QUERY,
    SELECT,
    DELETE,
    UPDATE,
};

#define get_sql_flag(type)                          \
    {                                               \
        type == SQL_OPER_TYPE::QUERY    ? "query"   \
        : type == SQL_OPER_TYPE::SELECT ? "select"  \
        : type == SQL_OPER_TYPE::DELETE ? "delete"  \
        : type == SQL_OPER_TYPE::UPDATE ? "update"  \
                                        : "unknown" \
    }

class DBProxy {
public:
    /**
     * ~DBProxy 默认析构函数
     *
     */
    ~DBProxy();

    /**
     * @brief: 执行数据库操作（insert、delete、update）
     * @author: yangchen
     * @param[in] sql 数据库执行语句
     * @param[in] index 数据库编号，DBConnConfig::index 确定，非负整数固定且有序（预留）
     * @param[out] ret_code 返回错误码
     * @return 返回执行结果 >= 0 返回更新的记录数，-1失败
     */
    int Exec(const std::string& sql, SQLError& ret_code, uint16_t index = 0);

    /**
     * @brief: 数据库预处理
     * @author: yangchen
     * @param[in] sql 数据库执行语句
     * @param[in] index 数据库编号，DBConnConfig::index 确定，非负整数固定且有序（预留）
     * @param[out] ret_code 返回错误码
     * @return 预处理结果
     */
    std::unique_ptr<PreparedStatement>
    Prepared(const std::string& sql, SQLError& ret_code, uint16_t index = 0);

    /**
     * @brief: 数据库查询
     * @author: yangchen
     * @param[in] sql 数据库查询语句
     * @param[in] index 数据库编号，DBConnConfig::index 确定，非负整数固定且有序（预留）
     * @param[out] ret_code 返回错误码
     * @return 查询结果集
     */
    std::unique_ptr<ResultSet>
    Query(const std::string& sql, SQLError& ret_code, uint16_t index = 0);

private:
    /**
     * @brief: 获取一个空闲的db连接
     * @author: yangchen
     * @param[in] times 尝试次数（获取数据库连接失败的时候重新尝试获取连接的次数，默认三次）
     * @param[out] 无
     * @return DB连接
     */
    BaseDBPtr GetDB(int times = 3);

    /**
     * @brief: 检验sql语句是否为type类型
     * @author: yangchen
     * @param[in] strsql 需要校验的sql语句
     * @param[in] type 需要校验的sql语句的类型，参考SQL_OPER_TYPE
     * @param[out] 无
     * @return true合法，false非法
     */
    bool CheckSql(std::string& strsql, const SQL_OPER_TYPE& type);
};

}  // namespace sql
#endif
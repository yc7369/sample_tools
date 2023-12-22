/*
 * @FilePath: basedb.h
 * @Brief:
 * @Version: 1.0
 * @Date: 2020-6-6 10:41:42
 * @Author: yangchen@hongkingsystem.cn
 * @Copyright: Copyright (c) 2021 LeHighHongKing All rights reserved.
 * @LastEditors: yangchen@hongkingsystem.cn
 * @LastEditTime: 2021-10-11 11:13:58
 */

#ifndef LHSERVER_BASEDB_H
#define LHSERVER_BASEDB_H

#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>

#include <memory>
#include <string>

namespace sql {

enum class SQLError {
    SUCCESS,
    NOT_CONNECTED,
    EMPTY_SQL,
    SQL_ERROR,
    EMPTY_CONNECTION,
    TIMEOUT,
    STATMENT_NULL,
    MYSQL_SERVER_GONE_AWAY,
    SQL_EXCEPTION,
};

// 抽象的数据库基类
class BaseDB {
public:
    /**
     * @brief: 打开数据库连接
     * @author: yangchen@hongkingsystem.cn
     * @param[in] addr 数据库地址，如：127.0.0.1:3306
     * @param[in] user 数据库用户名
     * @param[in] passwd 数据库密码
     * @param[in] db 连接的数据库名
     * @param[out] 无
     * @return true 成功，false 失败
     */
    virtual bool Open(const std::string& addr,
                      const std::string& user,
                      const std::string& passwd,
                      const std::string& db) = 0;

    /**
     * @brief: 关闭连接
     * @author: yangchen@hongkingsystem.cn
     * @param[in] 无
     * @param[out] 无
     * @return 无
     */
    virtual void Close() = 0;

    /**
     * @brief:  执行sql
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql sql语句
     * @param[out] ret_code 执行结果
     * @return 返回更新的记录条数
     */
    virtual int Exec(const std::string& sql, SQLError& ret_code) = 0;

    /**
     * @brief: 处理prepared statement
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql sql语句
     * @param[out] ret_code 执行结果
     * @return
     */
    virtual std::unique_ptr<PreparedStatement> Prepared(const std::string& sql,
                                                        SQLError& ret_code) = 0;

    /**
     * @brief: 执行查询，返回结果
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql 查询的sql
     * @param[out] ret_code 执行结果
     * @return 返回结果集
     */
    virtual std::unique_ptr<ResultSet> Query(const std::string& sql, SQLError& ret_code) = 0;

    virtual uint64_t GetDbSn() = 0;

    virtual bool IsOpen() {
        return open_;
    };

    /// 重新连接
    virtual bool Reconnect() = 0;
    virtual bool IsValid() = 0;

    /**
     * 设置是否自动提交
     * @author: yangchen@hongkingsystem.cn
     * @param[in] bflag 是否自动提交，true是  false否
     * @param[out] 无
     * @return 返回设置结果 0成功 非0失败
     */
    virtual int AutoCommit(const bool bflag = true) = 0;

    virtual int BeginTrans() = 0;

    /**
     * 开始事务处理
     * @author: yangchen@hongkingsystem.cn
     * @param[in] 无
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int SetSavePoint() = 0;

    /**
     * 提交事务
     * @author: yangchen@hongkingsystem.cn
     * @param[in] 无
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int Commit() = 0;

    /**
     * 事务回滚
     * @author: yangchen@hongkingsystem.cn
     * @param[in] 无
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int Rollback(bool flag = false) = 0;

protected:
    std::string addr_;
    std::string user_;
    std::string passwd_;
    std::string db_;
    uint64_t db_sn_;
    bool open_;
};

}  // namespace sql

#endif  // LHSERVER_BASEDB_H

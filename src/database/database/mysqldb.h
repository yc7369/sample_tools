/*
 * @FilePath: /lh_framework/include/database/mysqldb.h
 * @Brief: mysql操作类
 * @Version: 1.0
 * @Date: 2020-06-06 11:13:58
 * @Author: yangchen@hongkingsystem.cn
 * @Copyright: Copyright (c) 2021 LeHighHongKing All rights reserved.
 * @LastEditors: yangchen@hongkingsystem.cn
 * @LastEditTime: 2021-10-11 11:31:15
 */

#ifndef LHSERVER_MYSQLDB_H
#define LHSERVER_MYSQLDB_H

#include <cppconn/connection.h>

#include <atomic>
#include <boost/scoped_ptr.hpp>

#include "basedb.h"
#include "mysql/mysql_connection.h"
#include "mysql/mysql_driver.h"
namespace sql {

//
class MysqlDB : public BaseDB {
public:
    MysqlDB();
    virtual ~MysqlDB();

    /**
     * @brief: 打开数据库
     * @author: yangchen@hongkingsystem.cn
     * @param[in] addr 数据库地址
     * @param[in] user 数据库登录用户名
     * @param[in] passwd 数据库密码
     * @param[in] db 数据库名
     * @param[out] 无
     * @return true成功 false失败
     */
    virtual bool Open(const std::string& addr,
                      const std::string& user,
                      const std::string& passwd,
                      const std::string& db);

    /**
     * @brief: 关闭数据库
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * @param[out]
     * @return
     */
    virtual void Close();

    /**
     * @brief: 执行sql
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql sql语句
     * @param[out] ret_code 数据库执行结果返回错误码
     * @return 返回记录数
     */
    virtual int Exec(const std::string& sql, SQLError& ret_code);

    /**
     * @brief: 处理prepared statement
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql sql语句
     * @param[out] ret_code 错误码
     * @return 执行结果
     */
    virtual std::unique_ptr<PreparedStatement> Prepared(const std::string& sql, SQLError& ret_code);

    /**
     * @brief: 查询
     * @author: yangchen@hongkingsystem.cn
     * @param[in] sql 查询语句
     * @param[out] ret_code 返回错误码
     * @return 查询的结果集
     */
    virtual std::unique_ptr<ResultSet> Query(const std::string& sql, SQLError& ret_code);

    /**
     * @brief: 获取数据库标识
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * @param[out]
     * @return 数据库标识
     */
    virtual uint64_t GetDbSn();

    /**
     * @brief: 判断数据库是否打开
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * @param[out]
     * @return true打开 false关闭
     */
    virtual bool IsOpen();

    /**
     * @brief: 重连
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * @param[out]
     * @return true成功 false失败
     */
    virtual bool Reconnect();

    /**
     * @brief: 判断数据库是否可用
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * @param[out]
     * @return true可用 false不可用
     */
    virtual bool IsValid();

    /**
     * 设置是否自动提交
     * @author: yangchen@hongkingsystem.cn
     * @param[in] bflag 是否自动提交，true是  false否
     * @param[out] 无
     * @return 返回设置结果 0成功 非0失败
     */
    virtual int AutoCommit(const bool bflag = true);

    virtual int BeginTrans();

    /**
     * 设置事务点，每次会重置前一个事务点
     * @author: yangchen@hongkingsystem.cn
     * @param[in] point 事务点标识，同一次事务处理过程中必须唯一
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int SetSavePoint();

    /**
     * 提交事务
     * @author: yangchen@hongkingsystem.cn
     * @param[in] 无
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int Commit();

    /**
     * 事务回滚
     * @author: yangchen@hongkingsystem.cn
     * @param[in]
     * flag是否回滚到上一次保存的事务点，true回滚到上一个事务点，false全部回滚
     * @param[out] 无
     * @return 0成功 非0失败
     */
    virtual int Rollback(bool flag = false);

private:
    int MysqlExec(const std::string& sql, SQLError& ret_code);
    std::unique_ptr<PreparedStatement> MysqlPrepared(const std::string& sql, SQLError& ret_code);
    std::unique_ptr<ResultSet> MysqlQuery(const std::string& sql, SQLError& ret_code);
    int MysqlAutoCommit(const bool bflag, SQLError& ret_code);
    int MysqlSetSavePoint();
    int MysqlCommit(SQLError& ret_code);
    int MysqlRollback(bool flag, SQLError& ret_code);

    std::shared_ptr<Connection> connection_;
    boost::scoped_ptr<sql::Savepoint> point_;
    std::atomic_bool is_begin_trans_ = ATOMIC_FLAG_INIT;
    int idx_ = 0;
};

}  // namespace sql
#endif  // LHSERVER_MYSQLDB_H

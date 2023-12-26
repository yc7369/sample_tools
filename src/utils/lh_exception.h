/*
 * @FilePath: utils/lh_exception.h
 * @Brief: 自定义异常处理类
 * @Version: 1.0
 * @Date: 2020-10-27 16:53:30
 * @Author: yangchen
 * @Copyright: Copyright (c) 2021  All rights reserved.
 * @LastEditors: yangchen
 * @LastEditTime: 2021-10-12 10:15:09
 */

#ifndef __LH_EXCEPTION_H__
#define __LH_EXCEPTION_H__

#include <exception>
#include <string>

namespace lhserver {
class lh_exception : public std::exception {
public:
    lh_exception() : message_() {}

    /**
     * @brief: 带参构造函数
     * @author: yangchen
     * @param[in] message 异常信息
     */
    lh_exception(const std::string& message) : message_(message) {}

    virtual ~lh_exception() throw() {}

    /**
     * @brief: 获取异常信息
     * @author: yangchen
     * @return 异常信息
     */
    virtual const char* what() const throw() {
        if (message_.empty()) {
            return "default exception.";
        } else {
            return message_.c_str();
        }
    }

protected:
    std::string message_;  // 异常信息
};
}  // namespace lhserver
#endif
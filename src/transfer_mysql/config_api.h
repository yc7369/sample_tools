#pragma once
#include <array>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

struct MysqlInfo {
    std::string url_ = "tcp://172.24.13.178:3306";  // 数据库ip地址
    std::string user_ = "chronos";                  // 数据库用户名
    std::string pass_ = "chronos";                  // 数据库密码
    std::string data_base_ = "yctest";              // 数据库名字
};

struct BaseOption {
    // log
    int32_t log_level = 1;
    std::string log_dir = "./logs";
};

struct ConfigOpt {
    BaseOption base_;
    MysqlInfo dbinfo_;
};

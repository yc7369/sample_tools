# sample_tools
个人使用的简单工具
git@github.com:yc7369/sample_tools.git

# 编译环境
wsl Ubuntu 22.04.2
gcc 11.4
cmake 3.22.1

# 目录说明
src 测试或者工具以及基础轮子
infrbase 常用的工具类头文件
deps 可选的编译依赖库(源码)
    libvent
    hiredis
    redispp：redis++
    openssl
    spdlog    
thirdlib 第三方库的一些安装源码包


# 外层文件说明
.gitlab-ci.yml 配置git ci自动化编译发布配置,发布通过tag触发（这里可用作参考，未在该项目中生效）


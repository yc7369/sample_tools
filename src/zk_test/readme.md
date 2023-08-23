# 依赖库安装
## ubuntu22.04
sudo apt-get install libzookeeper-mt-dev

## centos 
yum install zookeeper-devel
安装不了就需要编译安装 源代码包：https://zookeeper.apache.org/releases.html 


# 文件说明

## zkfind.cpp
 简单的服务注册于发现的例子

## zklock.cpp 
简单的基于临时顺序节点实现的分布式锁例子
注：当客户端与zk创建的session失效后，zk也会删除掉创建的临时顺序节点，这样其他客户端就可以获取分布式锁了
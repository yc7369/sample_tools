#include <iostream>
#include <string>
#include <zookeeper/zookeeper.h>

// 定义全局 ZooKeeper 客户端句柄
zhandle_t *zh = nullptr;

// 用于监视锁节点变化的 Watcher
void lockWatcher(zhandle_t *zzh, int type, int state, const char *path, void *context) {
    if (type == ZOO_CREATED_EVENT) {
        std::cout << "Lock acquired!" << std::endl;
    }
}

bool acquireLock(const std::string &lockPath) {
    // 创建临时有序节点作为锁节点
    std::string value("lock-"); 
    int flags = ZOO_EPHEMERAL | ZOO_SEQUENCE;
    char buff[512];
    auto ret = zoo_create(zh, lockPath.c_str(), value.c_str(), value.length(), nullptr, flags, buff, sizeof(buff) - 1);
    if (ret) {
        std::cerr << "Failed to create lock node." << std::endl;
        return false;
    }

    // 获取所有子节点
    struct String_vector nodes;
    int rc = zoo_get_children(zh, "/locks", 0, &nodes);
    if (rc != ZOK) {
        std::cerr << "Failed to get children nodes." << std::endl;
        return false;
    }

    // 获取所有子节点中最小的节点
    std::string smallestNode = nodes.data[0];
    for (int i = 0; i < nodes.count; ++i) {
        if (nodes.data[i] < smallestNode) {
            smallestNode = nodes.data[i];
        }
    }

    // 如果创建的节点是最小的节点，则获得锁
    if (buff == "/locks/" + smallestNode) {
        std::cout << "Lock acquired!" << std::endl;
        return true;
    }

    // 否则，监视前一个节点的变化
    std::string l("/locks/");
    std::string prevNode = l + nodes.data[nodes.count - 2];
    rc = zoo_awget(zh, prevNode.c_str(), lockWatcher, nullptr, nullptr, nullptr);
    if (rc != ZOK) {
        std::cerr << "Failed to set watch on previous node." << std::endl;
        return false;
    }

    return false;
}

int main() {
    // 初始化 ZooKeeper 客户端
    zh = zookeeper_init("localhost:2181", nullptr, 30000, nullptr, nullptr, 0);
    if (!zh) {
        std::cerr << "Failed to initialize ZooKeeper client." << std::endl;
        return -1;
    }

    // 创建 /locks 节点
    int rc = zoo_create(zh, "/locks", nullptr, 0, &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);
    if (rc != ZOK && rc != ZNODEEXISTS) {
        std::cerr << "Failed to create /locks node." << std::endl;
        return -1;
    }

    std::string lockPath = "/locks/lock";
    bool locked = acquireLock(lockPath);

    // 在获得锁后执行需要锁保护的代码
    if (locked) {
        // 模拟工作
        std::cout << "Doing some work..." << std::endl;

        // 释放锁
        rc = zoo_delete(zh, lockPath.c_str(), -1);
        if (rc != ZOK) {
            std::cerr << "Failed to release lock." << std::endl;
        }
    }

    // 关闭 ZooKeeper 客户端
    zookeeper_close(zh);

    return 0;
}

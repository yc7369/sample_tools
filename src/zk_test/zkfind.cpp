#include <iostream>
#include <string>
#include <vector>
#include <zookeeper/zookeeper.h>

// 定义全局 ZooKeeper 客户端句柄
zhandle_t *zh = nullptr;

// 用于监视服务节点变化的 Watcher
void serviceWatcher(zhandle_t *zzh, int type, int state, const char *path, void *context) {
    if (type == ZOO_CHILD_EVENT) {
        // 获取当前服务节点的子节点（即可用的服务实例）
        struct String_vector nodes;
        int rc = zoo_get_children(zh, path, 1, &nodes);
        if (rc == ZOK) {
            std::cout << "Available service instances: ";
            for (int i = 0; i < nodes.count; ++i) {
                std::cout << nodes.data[i] << " ";
            }
            std::cout << std::endl;
        }
    }
}

void watchServices(const std::string &basePath) {
    // 设置服务节点的 Watcher
    int rc = zoo_awget_children(zh, basePath.c_str(), serviceWatcher, nullptr, nullptr, nullptr);
    if (rc != ZOK) {
        std::cerr << "Failed to set service watcher." << std::endl;
    }
}

int main() {
    // 初始化 ZooKeeper 客户端
    zh = zookeeper_init("localhost:2181", nullptr, 30000, nullptr, nullptr, 0);
    if (!zh) {
        std::cerr << "Failed to initialize ZooKeeper client." << std::endl;
        return -1;
    }

    // 假设服务实例的信息
    std::string serviceName = "my-service";
    std::string serviceHost = "192.168.1.100";
    int servicePort = 8080;

    // 创建服务实例的节点路径
    std::string serviceInstancePath = "/services/" + serviceName + "-" + serviceHost + ":" + std::to_string(servicePort);

    // 创建临时节点作为服务实例注册
    int rc = zoo_create(zh, serviceInstancePath.c_str(), nullptr, 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, nullptr, 0);
    if (rc != ZOK) {
        std::cerr << "Failed to register service instance." << std::endl;
        return -1;
    }

    // 监视服务节点变化
    watchServices("/services");

    // 模拟服务的工作
    std::cout << "Service is running..." << std::endl;

    // 在这里执行其他服务逻辑

    // 关闭 ZooKeeper 客户端
    zookeeper_close(zh);

    return 0;
}

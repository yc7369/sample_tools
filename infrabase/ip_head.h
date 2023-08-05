#include <cstdint>

// IP 头部结构体
struct IPHeader {
    uint8_t version_ihl;    // 版本号和头部长度
    uint8_t dscp_ecn;        // DSCP 和 ECN 字段
    uint16_t totalLength;   // 总长度
    uint16_t identification; // 标识符
    uint16_t flags_fragOffset; // 标志位和分段偏移
    uint8_t ttl;             // 存活时间
    uint8_t protocol;        // 协议类型 (TCP: 6, UDP: 17)
    uint16_t checksum;       // 校验和
    uint32_t srcIP;          // 源 IP 地址
    uint32_t destIP;         // 目标 IP 地址
};

// TCP 头部结构体
struct TCPHeader {
    uint16_t srcPort;        // 源端口
    uint16_t destPort;       // 目标端口
    uint32_t sequenceNum;    // 序列号
    uint32_t ackNum;         // 确认号
    uint8_t dataOffset;      // 数据偏移 (头部长度)
    uint8_t flags;           // 标志位 (URG, ACK, PSH, RST, SYN, FIN)
    uint16_t windowSize;     // 窗口大小
    uint16_t checksum;       // 校验和
    uint16_t urgentPointer;  // 紧急指针
};

// UDP 头部结构体
struct UDPHeader {
    uint16_t srcPort;        // 源端口
    uint16_t destPort;       // 目标端口
    uint16_t length;         // 长度 (头部 + 数据)
    uint16_t checksum;       // 校验和
};

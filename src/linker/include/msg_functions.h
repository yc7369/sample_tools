#pragma once

#include <stdint.h>

namespace linker {
#pragma pack(push)
#pragma pack(1)

#define DecodeMsg(type)                                                                        \
    type request;                                                                              \
    if (head.datalen != sizeof(type)) {                                                        \
        ZLOG_ERROR("{} sizeof " #type "({}) != {}", (int64_t)bev, sizeof(type), head.datalen); \
        evbuffer_drain(input, head.datalen);                                                   \
        break;                                                                                 \
    }                                                                                          \
    evbuffer_remove(input, &request, sizeof(type));

}  // namespace linker

#pragma pack(pop)

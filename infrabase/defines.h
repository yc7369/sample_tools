#pragma once

#ifndef DEFINE_SHARED_PTR
#include <memory>
#define DEFINE_SHARED_PTR(typ) typedef std::shared_ptr<typ> typ##Ptr;
#endif

#if defined(__GNUC__)
 inline bool(likely)(bool x) {
    return __builtin_expect((x), true);
}
 inline bool(unlikely)(bool x) {
    return __builtin_expect((x), false);
}
#else
 inline bool(likely)(bool x) {
    return x;
}
 inline bool(unlikely)(bool x) {
    return x;
}
#endif
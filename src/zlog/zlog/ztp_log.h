//
// Created by wen on 2017-10-13.
//

#ifndef ZTP_ZTP_LOG_H
#define ZTP_ZTP_LOG_H

#ifdef SPDLOG_ACTIVE_LEVEL
#undef SPDLOG_ACTIVE_LEVEL
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LOGGER_TRACE
#include "spdlog/spdlog.h"
#include <iostream>
#include <memory>
#include <string>

#ifndef ZTPLOG_DEF_H
#define ZTPLOG_DEF_H

#ifdef _WIN32
#ifdef ZTP_DLL
#ifdef ZTP_LIBRARY_EXPORTS
#define ZLOG_API __declspec(dllexport)
#else
#define ZLOG_API __declspec(dllimport)
#endif
#else
#define ZLOG_API
#endif
#else
#define ZLOG_API
#endif

#endif

namespace ztp {
void setup_log_system(const std::string& dir,
                      const std::string& filename,
                      int loglevel,
                      std::string loggername,
                      bool show_console = false);
void SlogInit(std::string dir, std::string logger_name, int32_t level);

void shutdown_log_system();

template <typename FormatString, typename... Args>
void inline LoG(const char* loggername,
                spdlog::level::level_enum level,
                const FormatString& fmt,
                Args&&... args) {
    const auto p = spdlog::get(loggername);
    if (p == nullptr) {
        // setup_log_system("./logs", loggername, spdlog::level::info, loggername, false);
        return;
    }
    p->log(level, fmt, std::forward<Args>(args)...);
}

void SlogInit(std::string dir, std::string logger_name, int32_t level);

}  // namespace ztp

#define _ZLOG_TRACE(logger, ...) ztp::LoG(logger, spdlog::level::trace, __VA_ARGS__)
#define _ZLOG_DEBUG(logger, ...) ztp::LoG(logger, spdlog::level::debug, __VA_ARGS__)
#define _ZLOG_INFO(logger, ...) ztp::LoG(logger, spdlog::level::info, __VA_ARGS__)
#define _ZLOG_WARN(logger, ...) ztp::LoG(logger, spdlog::level::warn, __VA_ARGS__)
#define _ZLOG_ERROR(logger, ...) ztp::LoG(logger, spdlog::level::err, __VA_ARGS__)
#define _ZLOG_CRITICAL(logger, ...) ztp::LoG(logger, spdlog::level::critical, __VA_ARGS__)

#define BASE_LOGGER_NAME "base"

#define BLOG_TRACE(...) _ZLOG_TRACE(BASE_LOGGER_NAME, __VA_ARGS__)
#define BLOG_DEBUG(...) _ZLOG_DEBUG(BASE_LOGGER_NAME, __VA_ARGS__)
#define BLOG_INFO(...) _ZLOG_INFO(BASE_LOGGER_NAME, __VA_ARGS__)
#define BLOG_WARN(...) _ZLOG_WARN(BASE_LOGGER_NAME, __VA_ARGS__)
#define BLOG_ERROR(...) _ZLOG_ERROR(BASE_LOGGER_NAME, __VA_ARGS__)
#define BLOG_CRITICAL(...) _ZLOG_CRITICAL(BASE_LOGGER_NAME, __VA_ARGS__)

#define ZLOG_TRACE(...) BLOG_TRACE(__VA_ARGS__)
#define ZLOG_DEBUG(...) BLOG_DEBUG(__VA_ARGS__)
#define ZLOG_INFO(...) BLOG_INFO(__VA_ARGS__)
#define ZLOG_WARN(...) BLOG_WARN(__VA_ARGS__)
#define ZLOG_ERROR(...) BLOG_ERROR(__VA_ARGS__)
#define ZLOG_CRITICAL(...) BLOG_CRITICAL(__VA_ARGS__)

#endif  // ZTP_ZTP_LOG_H

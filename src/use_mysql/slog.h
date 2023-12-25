#pragma once
#ifdef SPDLOG_ACTIVE_LEVEL
#undef SPDLOG_ACTIVE_LEVEL
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LOGGER_TRACE

#include "spdlog/spdlog.h"
#include <string>

inline std::string BASE_DATA_LOGGER_NAME = "transfer_cfetsmysql";
void SlogInit(std::string dir, int32_t level);

#define BSLOG_TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)
#define BSLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)
#define BSLOG_INFO(...) SPDLOG_LOGGER_INFO(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)
#define BSLOG_WARN(...) SPDLOG_LOGGER_WARN(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)
#define BSLOG_ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)
#define BSLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(BASE_DATA_LOGGER_NAME), __VA_ARGS__)

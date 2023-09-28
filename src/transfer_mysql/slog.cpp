#include "slog.h"
#include "zlog/ztp_log.h"

void SlogInit(std::string dir, int32_t level) {
    ztp::setup_log_system(dir, BASE_LOGGER_NAME, level, BASE_LOGGER_NAME, true);
    spdlog::get(BASE_LOGGER_NAME)->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");
    ztp::setup_log_system(dir, BASE_DATA_LOGGER_NAME, level, BASE_DATA_LOGGER_NAME, true);
    spdlog::get(BASE_DATA_LOGGER_NAME)->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");
}
//
// Created by wen on 2017-10-13.
//

#include "ztp_log.h"
#include "util.h"

#ifndef _WITHOUT_SPDLOG_
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/async.h"
#include "spdlog/common.h"
#endif

namespace ztp {

void shutdown_log_system() {
    spdlog::drop_all();
}

void setup_log_system(const std::string& dir,
                      const std::string& filename,
                      int loglevel,
                      std::string loggername,
                      bool show_console) {
    static std::atomic_bool inited = false;
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (!inited) {
        spdlog::init_thread_pool(81920, 2);
        spdlog::set_error_handler(
            [](const std::string& msg) { printf("*** LOGGER ERROR ***: %s\n", msg.c_str()); });
        spdlog::flush_every(std::chrono::seconds(2));
        inited = true;
    }

    if (spdlog::get(loggername))
        return;
    try {

        if (util::CreateDir(dir.c_str()) != 0) {
            std::cout << "mkdir log dir error: " << filename << std::endl;
            return;
        }
        auto path = dir + "/" + filename + ".log";

        std::vector<spdlog::sink_ptr> sinks;

        if (show_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(console_sink);
        }

        sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(path, 0, 0));
        auto combined_logger =
            std::make_shared<spdlog::async_logger>(loggername,
                                                   sinks.begin(),
                                                   sinks.end(),
                                                   spdlog::thread_pool(),
                                                   spdlog::async_overflow_policy::block);
        combined_logger->set_level(spdlog::level::level_enum(loglevel));
        combined_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");
        spdlog::register_logger(combined_logger);

    } catch (const spdlog::spdlog_ex& e) {
        std::cerr << e.what() << '\n';
    }
}

}  // namespace ztp


//
// Created by wen on 2017-10-13.
//

#ifndef QTPBASE_QTP_LOG_H
#define QTPBASE_QTP_LOG_H

#ifdef _SPD_LOGS_

#include <iostream>
#include <sstream>

#include "utils/qtp_util.h"
#include "spdlog/spdlog.h"
#include "utils/define.h"

// #include "utils/base_time.h"

namespace qtp {

enum level_x {
    kTrace = 0,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kCritical,
    kOff,
};

class LHHJ_API QtpLog {
public:
    typedef spdlog::level::level_enum level_enum;

    std::string g_log_dir;
    std::shared_ptr<spdlog::logger> g_default_log;
    int g_verbose;

    static QtpLog* Instance();

    void SetVerbose(int v);
    void SetLevel(level_enum lev);

    int Create(const std::string& app_name, const std::string& log_dir = "", int max_size = 100);
    void Shutdown();

    std::shared_ptr<spdlog::logger> Get(const std::string& log_name);
    std::shared_ptr<spdlog::logger> Get();
    ~QtpLog();

private:
    QtpLog();
};

}  // namespace qtp

#define QTPLOG_TRY try
#define QTPLOG_CATCH                                           \
    catch (const spdlog::spdlog_ex& ex) {                      \
        std::cout << "Log failed: " << ex.what() << std::endl; \
    }

#ifdef _WIN32
#define filename(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else
#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

#define LOG(level, ...) qtp::QtpLog::Instance()->Get()->level(__VA_ARGS__)
#define LOG_IF(level, pred, ...) \
    if (pred)                    \
    LOG(level, __VA_ARGS__)
#define LOG_TRACE(...) qtp::QtpLog::Instance()->g_default_log->trace(__VA_ARGS__)
#define LOG_DEBUG(...) qtp::QtpLog::Instance()->g_default_log->debug(__VA_ARGS__)
#define LOG_INFO(...) qtp::QtpLog::Instance()->g_default_log->info(__VA_ARGS__)
#define LOG_WARN(...) qtp::QtpLog::Instance()->g_default_log->warn(__VA_ARGS__)
#define LOG_ERROR(...) qtp::QtpLog::Instance()->g_default_log->error(__VA_ARGS__)
#define LOG_CRITICAL(...) qtp::QtpLog::Instance()->g_default_log->critical(__VA_ARGS__)
#define VLOG(v, ...) \
    qtp::QtpLog::Instance()->Get()->info_if((v) <= qtp::QtpLog::Instance()->g_verbose, __VA_ARGS__)

#define DEBUG

#ifdef DEBUG

#define QTP_LOG(level, log) \
    LOG(level, "[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_IF(log) LOG_IF("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_TRACE(log) \
    LOG_TRACE("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_DEBUG(log) \
    LOG_DEBUG("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_INFO(log) \
    LOG_INFO("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_WARN(log) \
    LOG_WARN("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_ERROR(log) \
    LOG_ERROR("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)
#define QTP_LOG_CRITICAL(log) \
    LOG_CRITICAL("[{}:{}][{}] {}", filename(__FILE__), __LINE__, __FUNCTION__, log)

#else

#define QTP_LOG(level, log) LOG(level, "{}", log)
#define QTP_LOG_IF(log) LOG_IF("{}", log)
#define QTP_LOG_TRACE(log) LOG_TRACE("{}", log)
#define QTP_LOG_DEBUG(log) LOG_DEBUG("{}", log)
#define QTP_LOG_INFO(log) LOG_INFO("{}", log)
#define QTP_LOG_WARN(log) LOG_WARN("{}", log)
#define QTP_LOG_ERROR(log) LOG_ERROR("{}", log)
#define QTP_LOG_CRITICAL(log) LOG_CRITICAL("{}", log)

#endif

#define HK_LOG(LEVEL, in_sstream)     \
    {                                 \
        std::ostringstream ostrm;     \
        ostrm << in_sstream;          \
        QTP_LOG_##LEVEL(ostrm.str()); \
    }

#define HK_LOG_IF(in_sstream) HK_LOG(IF, in_sstream)
#define HK_LOG_TRACE(in_sstream) HK_LOG(TRACE, in_sstream)
#define HK_LOG_DEBUG(in_sstream) HK_LOG(DEBUG, in_sstream)
#define HK_LOG_INFO(in_sstream) HK_LOG(INFO, in_sstream)
#define HK_LOG_WARN(in_sstream) HK_LOG(WARN, in_sstream)
#define HK_LOG_ERROR(in_sstream) HK_LOG(ERROR, in_sstream)
#define HK_LOG_CRITICAL(in_sstream) HK_LOG(CRITICAL, in_sstream)

#else

#include <stdarg.h>
#include <stdint.h>

#include <atomic>
#include <string>

#include "glog/log_severity.h"
#include "glog/logging.h"
namespace qtp {
enum VLOG_LEVEL {
    kTrace = 6,
    kDebug = 5,
    kInfo = 4,
    kWarn = 3,
    kError = 2,
    kCritical = 1,
    kOff = 0,
};

class GLogUtils {
public:
    GLogUtils(void);
    ~GLogUtils(void);
    int InitGLog(const char* appname,
                 const char* log_dir,
                 int max_file_size,
                 int32_t severity,
                 int32_t logbuflevel,
                 int level);
    void SetVlogLevel(int level);
    const std::string GetAppName() const;
    int mkdir_p(const char* log_dir);
    void ReleaseGlog();

public:
    static GLogUtils* Instance();

private:
    static GLogUtils* inst_;
    std::atomic<bool> is_log_inited_ = ATOMIC_FLAG_INIT;
    std::string appname_;
};
}  // namespace qtp
#ifdef _WIN32
#define GLOGFILENAME(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else
#define GLOGFILENAME(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

#define __GFILE__ (GLOGFILENAME(__FILE__))

#define HK_GLOG_UTILS_INIT(...) qtp::GLogUtils::Instance()->InitGLog(__VA_ARGS__)
#define HK_GLOG_UTILS_MODULE qtp::GLogUtils::Instance()->GetAppName()

#define HK_GLOG_UTILS_INFO LOG(INFO)
#define HK_GLOG_UTILS_WARN LOG(WARNING)
#define HK_GLOG_UTILS_ERROR LOG(ERROR)
#define HK_GLOG_UTILS_FATAL LOG(FATAL)

#define HK_GLOG_TRACE                                                                   \
    VLOG(qtp::VLOG_LEVEL::kTrace) << "[TRACE][" << __GFILE__ << ":" << __LINE__ << "][" \
                                  << __FUNCTION__ << "] "
#define HK_GLOG_DEBUG                                                                   \
    VLOG(qtp::VLOG_LEVEL::kDebug) << "[DEBUG][" << __GFILE__ << ":" << __LINE__ << "][" \
                                  << __FUNCTION__ << "] "
#define HK_GLOG_INFO                                                                  \
    VLOG(qtp::VLOG_LEVEL::kInfo) << "[INFO][" << __GFILE__ << ":" << __LINE__ << "][" \
                                 << __FUNCTION__ << "] "
#define HK_GLOG_WARN                                                                  \
    VLOG(qtp::VLOG_LEVEL::kWarn) << "[WARN][" << __GFILE__ << ":" << __LINE__ << "][" \
                                 << __FUNCTION__ << "] "
#define HK_GLOG_ERROR                                                                   \
    VLOG(qtp::VLOG_LEVEL::kError) << "[ERROR][" << __GFILE__ << ":" << __LINE__ << "][" \
                                  << __FUNCTION__ << "] "
#define HK_GLOG_CRITICAL             \
    VLOG(qtp::VLOG_LEVEL::kCritical) \
        << "[CRITICAL][" << __GFILE__ << ":" << __LINE__ << "][" << __FUNCTION__ << "] "
#define HK_GLOG_UTILS_RELEASE qtp::GLogUtils::Instance()->ReleaseGlog

#endif

#endif  // QTPBASE_QTP_LOG_H

#pragma once
// 因为这个文件有inline 同时对外提供的头文件是带前缀的在build路径,
// pragma once 只能以路径来识别同一文件，会导致重定义
#ifndef MYBASE_TIMEMAN
#define MYBASE_TIMEMAN

#include <functional>
#include <cstring>
#include <atomic>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

namespace mybase {
// timer回调函数，返回false的时候取消定时器
typedef std::function<void()> TimerCallback;
typedef void* TimerHandle;

// 定时器管理类，最高精度10ms
class TimerMan {
private:
    TimerMan();
    ~TimerMan();

public:
    static TimerMan* instance();

    // 设定定时器
    //  timerout_ms 定时器超时， 最小精度10ms
    //  cnt 执行次数，>= 1
    //  cb 定时器触发的时候的回调函数。 注意，此回调是在一个新的线程中调用的
    //  返回timer_id
    TimerHandle setTimer(int32_t timerout_ms, int32_t cnt, TimerCallback cb);

    // 取消定时器
    void stopTimer(TimerHandle timer);

    // 非阻塞获取当前ms时间戳
    // 必须run之后才能获取
    inline int64_t unixTimeNow() const {
        return time_ms_;
    }

    // 当前时间，格式 hhmmsssss
    inline int32_t timeNowHMSs() const {
        return time_hmss_;
    }

    // 当前日期，格式yyyymmdd
    inline int32_t dateNowYMD() const {
        return date_ymd_;
    }

    inline const struct tm* rawtm() const {
        return &pstm_;
    }

    void run();

private:
    // pimpl
    void* impl_ = nullptr;
    // unix time
    int64_t time_ms_;
    // yyyymmdd
    int32_t date_ymd_;
    // HHMMSSsss
    int32_t time_hmss_;
    struct tm pstm_;
    std::atomic_bool is_stoped_{ false };
};

namespace time {

inline int64_t current_time_ns() {
#ifdef _MSC_VER
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    auto epoch = now_ns.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);
    return value.count();
#else

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        printf("clock_gettime failed: %s\n", strerror(errno));
        return -1;
    }

    return (ts.tv_sec * 1000000000) + ts.tv_nsec;
#endif  // _MSC_VER
}

// ymd : yyyymmdd : 20200203。为0的时候自动取当前日期
// hmss : HHMMSSsss，带有毫秒值, 112233444
inline int64_t makeUnixTimeMs(int32_t ymd, int32_t hms) {
    auto inst = TimerMan::instance();
    if (ymd <= 0) {
        ymd = inst->dateNowYMD();
    }

    struct tm t = { 0 };
    std::memcpy(&t, inst->rawtm(), sizeof(struct tm));
    t.tm_wday = 0;
    t.tm_mday = ymd % 100;
    ymd /= 100;
    t.tm_mon = ymd % 100 - 1;
    t.tm_year = ymd / 100 - 1900;

    int ms = hms % 1000;
    hms /= 1000;
    t.tm_sec = hms % 100;
    hms /= 100;
    t.tm_min = hms % 100;
    t.tm_hour = hms / 100;
    return int64_t(mktime(&t)) * 1000 + ms;
}

// 系统当前时间（unix时间戳），带有ms值
inline int64_t unixTimeNowMs() {
    return ::mybase::TimerMan::instance()->unixTimeNow();
}

static int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone) {
    static const int kHoursInDay = 24;
    static const int kMinutesInHour = 60;
    static const int kDaysFromUnixTime = 2472632;
    static const int kDaysFromYear = 153;
    static const int kMagicUnkonwnFirst = 146097;
    static const int kMagicUnkonwnSec = 1461;
    tm->tm_sec = unix_sec % kMinutesInHour;
    int i = (unix_sec / kMinutesInHour);
    tm->tm_min = i % kMinutesInHour;  // nn
    i /= kMinutesInHour;
    tm->tm_hour = (i + time_zone) % kHoursInDay;  // hh
    tm->tm_mday = (i + time_zone) / kHoursInDay;
    int a = tm->tm_mday + kDaysFromUnixTime;
    int b = (a * 4 + 3) / kMagicUnkonwnFirst;
    int c = (-b * kMagicUnkonwnFirst) / 4 + a;
    int d = ((c * 4 + 3) / kMagicUnkonwnSec);
    int e = -d * kMagicUnkonwnSec;
    e = e / 4 + c;
    int m = (5 * e + 2) / kDaysFromYear;
    tm->tm_mday = -(kDaysFromYear * m + 2) / 5 + e + 1;
    tm->tm_mon = (-m / 10) * 12 + m + 2;
    tm->tm_year = b * 100 + d - 6700 + (m / 10);
    return 0;
}

// unix_datetime: unix 时间戳带ms
// date : yyyymmdd : 20200203
// time : HHMMSSsss，带有毫秒值, 112233444
inline void
unixTimeToDateTime(const int64_t& unix_datetime, int32_t& date_yyyymmdd, int32_t& time_hhmmsssss) {
    struct tm tm;
    FastSecondToDate(unix_datetime / 1000, &tm, 8);  // 中国东8区,如有必要的时候这里可以弄个变量传递
    date_yyyymmdd = (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
    time_hhmmsssss =
        unix_datetime % 1000 + tm.tm_hour * 10000000 + tm.tm_min * 100000 + tm.tm_sec * 1000;
}

// 获取机器当前的HHMMSSsss值，带毫秒
inline int32_t currentHMSs() {
    return TimerMan::instance()->timeNowHMSs();
};

// 10:19:06.262 -> timems
inline int64_t partStrtime2unixALL(const char* strtime) {
    if (strtime == nullptr || strlen(strtime) < 12) {
        return unixTimeNowMs();
    }
    int hh = 0, mm = 0, ss = 0, sss = 0;
    sscanf(strtime, "%d:%d:%d.%d", &hh, &mm, &ss, &sss);
    int64_t hhmmss = hh * 10000000 + mm * 100000 + ss * 1000 + sss;
    int64_t ymd = TimerMan::instance()->dateNowYMD();
    return makeUnixTimeMs(ymd, hhmmss);
}

#ifdef _MSC_VER

inline tm StringToDatetime(const char* str) {
    tm tm_;
    int year, month, day, hour, minute, second;
    sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_.tm_year = year - 1900;
    tm_.tm_mon = month - 1;
    tm_.tm_mday = day;
    tm_.tm_hour = hour;
    tm_.tm_min = minute;
    tm_.tm_sec = second;
    tm_.tm_isdst = 0;

    return tm_;  // 秒时间
}

#endif  // _MSC_VER

// 2022-09-16 10:19:06.262
// ONLY China East 8 Zone
inline int64_t strtime2unix(const char* strtime) {
    if (strtime == NULL || strlen(strtime) < 19) {
        return 0;
    }
    struct tm tm {};
#ifdef _MSC_VER
    tm = StringToDatetime(strtime);
#else
    strptime(strtime, "%Y-%m-%d %H:%M:%S", &tm);
#endif
    int ms = 0;
    if (strlen(strtime) > 19) {
        ms = atoi(strtime + 20);
    }
    auto ret = int64_t(mktime(&tm)) * 1000 + ms;
    if (ret < 0) {
        auto now = unixTimeNowMs();
        std::cout << "strtime2unix error: " << strtime << " ms:" << now << std::endl;
        return now;
    }
    return ret;
}

// 2022-09-16 10:19:06.262
inline int64_t strtime2int64(const char* strtime) {
    if (strtime == NULL || strlen(strtime) < 19) {
        return 0;
    }
    int64_t ret = 0;
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, ms = 0;
    sscanf(strtime, "%d-%d-%d %d:%d:%d.%d", &year, &month, &day, &hour, &minute, &second, &ms);
    ret = year * 10000000000 + month * 100000000 + day * 1000000 + hour * 10000 + minute * 100 +
          second;
    return ret * 1000 + ms;
}
inline std::string unix2Hmss(int64_t unixtime) {
    struct tm tm;
    FastSecondToDate(unixtime / 1000, &tm, 8);
    char buf[32] = { 0 };
    snprintf(buf,
             sizeof(buf),
             "%02d:%02d:%02d.%03d",
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec,
             (int)(unixtime % 1000));
    return buf;
}

inline std::string unix2Full(int64_t unixtime) {
    struct tm tm;
    FastSecondToDate(unixtime / 1000, &tm, 8);
    char buf[32] = { 0 };
    snprintf(buf,
             sizeof(buf),
             "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             tm.tm_year + 1900,
             tm.tm_mon + 1,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec,
             (int)(unixtime % 1000));
    return buf;
}

}  // namespace time
}  // namespace mybase

#endif
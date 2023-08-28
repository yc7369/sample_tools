#include "timerman.h"
#include "timer-wheel.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <thread>
#include <unordered_set>
#include "util.h"
#ifndef _WIN64
#include <unistd.h>
#include <sys/timeb.h>
#else
#include <time.h>

#endif  //  _WIN64

#include <chrono>

#define IMPL(v) auto& v = (*((_TimerImpl*)(impl_)));
#define IMPL_LOCK(v) \
    IMPL(v)          \
    std::lock_guard<std::recursive_mutex> lock(v.m);

// 最小时间精度 10ms
static const int32_t MIN_RESOLUTION = 5;

namespace mybase {

class TMEvent {
public:
    TMEvent() : inc_timer_(this), canceled_(false) {}

    void start(TimerWheel* wheel, int32_t ms, int32_t cnt, TimerCallback cb) {
        wheel_ = wheel;
        ms_ = ms;
        cb_ = cb;
        max_cnts_ = cnt;
        wheel_->schedule(&inc_timer_, ms_);
    }

    void onTimer() {
        cb_();
        cnts_++;
        if (!canceled_ && (cnts_ < max_cnts_)) {
            wheel_->schedule(&inc_timer_, ms_);
        }
    }

    void cancel() {
        canceled_ = true;
        inc_timer_.cancel();
    }

private:
    MemberTimerEvent<TMEvent, &TMEvent::onTimer> inc_timer_;
    TimerWheel* wheel_ = nullptr;
    int32_t ms_;
    int32_t cnts_;
    int32_t max_cnts_;
    std::atomic_bool canceled_;
    TimerCallback cb_ = nullptr;
};

// typedef TimerEvent<TimerCallback> TMEvent;
// typedef std::shared_ptr<TMEvent> TMEventPtr;
typedef TMEvent* TMEventPtr;

struct _TimerImpl {
    _TimerImpl() : id(1) {}

    TimerWheel wheel;
    std::atomic_int64_t id;
    std::recursive_mutex m;
    // std::unordered_map<int64_t, TMEventPtr> timers;
    std::unordered_set<TMEventPtr> timers;

    TimerHandle set(int32_t ms, int32_t cnt, TimerCallback cb) {
        // std::lock_guard<std::recursive_mutex> lock(this->m);
        // id++;
        // auto evt = std::make_shared<TMEvent>(&wheel, ms, cnt, cb);
        auto evt = new TMEvent();
        // timers[id] = evt;
        timers.insert(evt);
        evt->start(&wheel, ms, cnt, cb);
        // wheel.schedule(evt.get(), ms);
        // wheel.schedule_in_range(evt.get(), ms, ms * 100000);
        return (TimerHandle)evt;
    }

    void cancel(TimerHandle htimer) {
        std::lock_guard<std::recursive_mutex> lock(this->m);
        if (TMEvent* evt = (TMEvent*)htimer) {
            evt->cancel();
            timers.erase(evt);
            delete evt;
        }
    }
};

TimerMan::TimerMan() {
    impl_ = new _TimerImpl();
    time_ms_ = 0;
}

TimerMan::~TimerMan() {
    is_stoped_ = true;
    if (impl_) {
        delete (_TimerImpl*)impl_;
        impl_ = nullptr;
    }
}

TimerMan* TimerMan::instance() {
    static TimerMan _inst;
    return &_inst;
}

TimerHandle TimerMan::setTimer(int32_t timerout_ms, int32_t cnt, TimerCallback cb) {
    IMPL_LOCK(d);
    return d.set(timerout_ms, cnt, cb);
}

void TimerMan::run() {
    std::thread([&]() {
        IMPL(v);
        while (!is_stoped_) {
            v.wheel.advance(MIN_RESOLUTION);
            std::this_thread::sleep_for(std::chrono::milliseconds(MIN_RESOLUTION));
        }
    }).detach();

    std::thread([&]() {
        struct timeb t_now = { 0 };
        while (!is_stoped_) {
            ftime(&t_now);
            time_ms_ = t_now.time * 1000 + t_now.millitm;

            auto now = std::chrono::system_clock::now();

            auto now_time = std::chrono::system_clock::to_time_t(now);

            // local time
#ifdef _MSC_VER
            localtime_s(&pstm_, &t_now.time);
#else
            localtime_r(&t_now.time, &pstm_);
#endif  // _MSC_VER

            // yyyymmdd
            date_ymd_ = (pstm_.tm_year + 1900) * 10000 + (pstm_.tm_mon + 1) * 100 + pstm_.tm_mday;
            // HHMMSSsss
            time_hmss_ = (pstm_.tm_hour) * 10000000 + pstm_.tm_min * 100000 + pstm_.tm_sec * 1000 +
                         t_now.millitm;
            // 经测试 100~400的时候相对合适，再增加睡眠时间对降低cpu占用的收效比较小

            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }).detach();
}

void TimerMan::stopTimer(TimerHandle htimer) {
    IMPL_LOCK(d);
    d.cancel(htimer);
}

}  // namespace mybase

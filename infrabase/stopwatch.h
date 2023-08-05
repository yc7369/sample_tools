#pragma once
#include <chrono>
using namespace std;
using namespace std::chrono;

typedef time_point<high_resolution_clock> TimePoint;
typedef time_point<high_resolution_clock> CheckPoint;

class StopWatch {
public:
    StopWatch() : _begin(high_resolution_clock::now()) {}
    void reset() {
        _begin = high_resolution_clock::now();
    }

    double seconds() const {
        return duration_cast<duration<double>>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed() const {  // milli seconds
        return duration_cast<chrono::milliseconds>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed_micro() const {
        return duration_cast<chrono::microseconds>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed_nano() const {
        return duration_cast<chrono::nanoseconds>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed_seconds() const {
        return duration_cast<chrono::seconds>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed_minutes() const {
        return duration_cast<chrono::minutes>(high_resolution_clock::now() - _begin).count();
    }

    int64_t elapsed_hours() const {
        return duration_cast<chrono::hours>(high_resolution_clock::now() - _begin).count();
    }

    TimePoint begin() {
        return _begin;
    }

    void restart() {
        _begin = high_resolution_clock::now();
    }

private:
    TimePoint _begin;
};
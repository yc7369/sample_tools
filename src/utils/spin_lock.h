
#ifndef QTPBASE_SPIN_LOCK_H
#define QTPBASE_SPIN_LOCK_H

#include <atomic>

class SpinLock {
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

private:
#ifndef _WIN32
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
#else
    std::atomic_flag flag_;
#endif
};

#endif  // QTPBASE_SPIN_LOCK_H


#ifndef QTPBASE_SINGLETON_H
#define QTPBASE_SINGLETON_H

#include "spin_lock.h"


template <typename T>
class Singleton : public SpinLock {
   public:
    inline static T* Instance() {  // it's thread-safe in c++11
        static T t;
        return &t;
    }

   protected:
    Singleton(void) {}
    ~Singleton(void) {}

   private:
    Singleton(const Singleton& rhs);
    Singleton& operator=(const Singleton& rhs);
};


#endif  // QTPBASE_SINGLETON_H

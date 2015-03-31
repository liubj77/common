// Filename: mutex.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_MUTEX_H_
#define  _COMMON_MUTEX_H_

#include <pthread.h>

namespace common {

class MutexLock {
public:
    MutexLock() {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
    }

    int trylock() {
        return pthread_mutex_trylock(&mutex_);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *mutex() {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
};

template<class T>
class MutexLockGuard {
public:
    explicit MutexLockGuard(T &mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    T &mutex_;
};

} /* namespace common */

#endif //MUTEX_H_


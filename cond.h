// Filename: cond.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_COND_H_
#define  _COMMON_COND_H_

#include <errno.h>
#include "common/mutex.h"

namespace common {

class Condition {
public:
    explicit Condition(MutexLock &mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

    void wait() {
        pthread_cond_wait(&cond_, mutex_.mutex());
    }

    /* return true if timeout */
    bool timedwait(struct timespec *timeout) {
        return ETIMEDOUT == 
            pthread_cond_timedwait(&cond_, mutex_.mutex(), timeout);
    }

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notify_all() {
        pthread_cond_broadcast(&cond_);
    }

private:
    MutexLock &mutex_;
    pthread_cond_t cond_;
};

}  /* namespace common */

#endif //COND_H_


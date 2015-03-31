// Filename: sema.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_SEMA_H_
#define  _COMMON_SEMA_H_

#include <semaphore.h>
#include <errno.h>

namespace common {

class Semaphore {
public:
    explicit Semaphore(int value = 0) {
        sem_init(&sema_, 0, value);
    }

    ~Semaphore() {
        sem_destroy(&sema_);
    }

    /* blocking call, until error occur */
    void wait() {
        sem_wait(&sema_);
    }

    /* return true, if sem was already locked */
    bool try_wait() {
        return EAGAIN == sem_trywait(&sema_);
    }

    void notify() {
        sem_post(&sema_);
    }

private:
    sem_t sema_;
};

} /* namespace common */

#endif //SEMA_H_


// Filename: queue.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_QUEUE_H_
#define  _COMMON_QUEUE_H_

#include <queue>
#include "common/mutex.h"
#include "common/cond.h"
#include "common/sema.h"

namespace common {

// blocking queue synced by condition
template<class T>
class BlockingQueue {
public:
    BlockingQueue() : 
        mutex_(), notify_cond_(mutex_) {   
    }

    ~BlockingQueue() {
    }

    void push_front(T *elem) {
        MutexLockGuard<MutexLock> lock(mutex_);
        deque_.push_front(elem);
        notify_cond_.notify();
    }
    
    void push_back(T *elem) {
        MutexLockGuard<MutexLock> lock(mutex_);
        deque_.push_back(elem);
        notify_cond_.notify();
    }

    T* pop_front() {
        MutexLockGuard<MutexLock> lock(mutex_);
        while (deque_.empty()) {
            // if empty, wait and while check
            notify_cond_.wait();
        }
        T *elem = deque_.front();
        deque_.pop_front();

        return elem;
    }

    T* pop_back() {
        MutexLockGuard<MutexLock> lock(mutex_);
        while (deque_.empty()) {
            notify_cond_.wait();
        }
        T *elem = deque_.back();
        deque_.pop_back();

        return elem;
    }

private:
    std::deque<T *> deque_;
    MutexLock mutex_;
    Condition notify_cond_;
};

// blocking queue synced by semaphore
template<class T>
class BlockingSemaQueue {
public:
    BlockingSemaQueue() {
    }

    ~BlockingSemaQueue() {
    }

    void push_front(T *elem) {
        MutexLockGuard<MutexLock> lock(mutex_);
        deque_.push_front(elem);
        sema_.notify();
    }

    void push_back(T *elem) {
        MutexLockGuard<MutexLock> lock(mutex_);
        deque_.push_back(elem);
        sema_.notify();
    }

    T* pop_front() {
        while (deque_.empty()) {
            sema_.wait();
        }

        T *elem = deque_.front();
        deque_.pop_front();
        
        return elem;
    }

    T *pop_back() {
        while (deque_.empty()) {
            sema_.wait();
        }

        T *elem = deque_.back();
        deque_.pop_back();

        return elem;
    }

private:
    std::deque<T *> deque_;
    MutexLock mutex_;
    Semaphore sema_;
};


} /* namespace common */

#endif //QUEUE_H_


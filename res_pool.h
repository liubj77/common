// Filename: res_pool.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_RES_POOL_H_
#define  _COMMON_RES_POOL_H_

#include "common/queue.h"

namespace common {

template <class ResType>
class ResourcePool {
public:
    ResourcePool(bool alive_check = false) :
        cur_size_(0),
        max_size_(0),
        alive_check_(alive_check) {    
    }

    virtual ~ResourcePool() {
        destroy_pool();
    }

    int init_pool(int size);
    void destroy_pool();

    /* below method can be overwrited by child class */
    virtual ResType *create_resource() = 0;
    virtual void destroy_resource(ResType *r) = 0;
    virtual int check_resource(ResType *r);

    ResType *get_resource();
    void release_resource(ResType *res);
    
private:
    int cur_size_;
    int max_size_;
    bool alive_check_;
    BlockingQueue<ResType> block_queue_;
};

#include "res_pool_inl.h"

} /* namespace common */

#endif //RES_POOL_H_


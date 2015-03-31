// Filename: res_pool_inl.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _RES_POOL_INL_H_
#define  _RES_POOL_INL_H_

namespace common {

template <class ResType>
int ResourcePool<ResType>::init_pool(int size)
{
    max_size_ = size;
    for (int i = 0; i < size/2; ++i) {
        ResType *r = create_resouce();
        if (r == NULL) {
            destroy_pool();
            return -1;
        }

        ++cur_size_;
        block_queue_.push_back(r);
    }

    return 0;
}

template <class ResType>
void ResourcePool<ResType>::destroy_pool()
{
    ResType *r = NULL;
    while (cur_size_ -- > 0) {
        r = block_queue_.pop_front();
        destroy_resource(r);
    }
}

template <class ResType>
int ResourcePool<ResType>::check_resource(ResType *r) 
{
    (void)r;
    return 0;
}

template <class ResType>
ResType *ResourcePool<ResType>::get_resource()
{
    ResType *r = NULL;
    if (block_queue_.size() > 0) {
        r = block_queue_.pop_front();
        if (alive_check_) {
            if (check_resource(r) != 0) {
                // not alive
                destroy_resource(r);
                r = create_resouce();
                if (r == NULL) {
                    -- cur_size_;
                }
            }
        }
    } else {
        if (cur_size_ < max_size_) {
            r = create_resouce();
            if (r) {
                ++ cur_size_;
            }
        }
    }

    return r;
}

template <class ResType>
void ResourcePool<ResType>::release_resource(ResType *r) 
{
    if (r) {
        block_queue_.push_back(r);
    }
}

} /* namespace common */

#endif //RES_POOL_INL_H_


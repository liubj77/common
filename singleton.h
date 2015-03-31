// Filename: singleton.h
// Author: liubj77 - liubj77@gmail.com

#ifndef  _COMMON_SINGLETON_H_
#define  _COMMON_SINGLETON_H_

#define DACLARE_SINGLETON(classname)        \
public:                                     \
    static classname * instance() {         \
        if (s_instance_ != NULL) {          \
            return s_instance_;             \
        }                                   \
                                            \
        if (s_instance_ == NULL) {          \
            s_instance_ = new classname();  \
        }                                   \
        return s_instance_;                 \
    }                                       \
protected:                                  \
    classname();                            \
    static classname *s_instance_;          \
private:

#define IMPLEMENT_SINGLETON(classname)      \
    classname * classname::s_instance_ = NULL;  \
    classname::classname() {}

#endif //SINGLETON_H_


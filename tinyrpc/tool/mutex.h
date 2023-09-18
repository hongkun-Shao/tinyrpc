#ifndef TINYRPC_TOOL_MUTEX_H
#define TINYRPC_TOOL_MUTEX_H

#include <pthread.h>

namespace tinyrpc{

//p_thread RAII class
class Mutex{
public:
    Mutex(){
        pthread_mutex_init(&m_mutex_, NULL);
    }
    ~Mutex(){
        pthread_mutex_destroy(&m_mutex_);
    }

    void lock(){
        pthread_mutex_lock(&m_mutex_);
    }

    void unlock(){
        pthread_mutex_unlock(&m_mutex_);
    }
private:
    pthread_mutex_t m_mutex_;
};

//Automatic locking, unlocking class
template <class T>
class Locker{
public:
    Locker(T & mutex) : m_mutex_(mutex){
        m_mutex_.lock();
        m_is_lock_ = true;
    }
    ~Locker(){
        m_mutex_.unlock();
        m_is_lock_ = false;
    }

    void lock() {
        if (!m_is_lock) {
            m_mutex_.lock();
        }
    }

    void unlock() {
        if (m_is_lock_) {
            m_mutex_.unlock();
        }
    }
private:
    T & m_mutex_;
    bool m_is_lock_;
};




}

#endif
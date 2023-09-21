#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/io_thread_pool.h"

namespace tinyrpc{

IOThreadPool::IOThreadPool(int num) : m_thread_num_(num){
    m_io_threads_pool_.resize(num);
    for(size_t i = 0; i < num; ++ i){
        m_io_threads_pool_[i] = new IOThread();
    }
}

IOThreadPool::~IOThreadPool(){

}

void IOThreadPool::Start(){
    for(size_t i = 0; i < m_io_threads_pool_.size(); ++ i){
        m_io_threads_pool_[i]->start();
    }
}

void IOThreadPool::Join(){
    for(size_t i = 0; i < m_io_threads_pool_.size(); ++ i){
        m_io_threads_pool_[i]->join();
    }
}

IOThread * IOThreadPool::get_iothread(){
    m_index_ %= m_io_threads_pool_.size();
    return m_io_threads_pool_[m_index_ ++];
}

}
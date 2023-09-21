#include <assert.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"
#include "tinyrpc/net/io_thread.h"

namespace tinyrpc{

IOThread::IOThread(){
    int res = sem_init(&m_init_sem_, 0, 0);
    assert(res == 0);

    res = sem_init(&m_start_sem_, 0, 0);
    assert(res == 0);

    pthread_create(&m_thread_, NULL, &IOThread::Main, this);

    // wait, avoid accessing eventloop before creation is completed
    sem_wait(&m_init_sem_);
    DEBUGLOG("IOThread [%d] create success", m_thread_id_);
}

IOThread::~IOThread(){
    m_event_loop_->Stop();
    sem_destroy(&m_init_sem_);
    sem_destroy(&m_start_sem_);

    pthread_join(m_thread_, NULL);

    if(m_event_loop_){
        delete m_event_loop_;
        m_event_loop_ = nullptr;
    }
}

EventLoop * IOThread::get_eventloop(){
    return m_event_loop_;
}

void IOThread::start(){
    DEBUGLOG("Now invoke IOThread %d", m_thread_id_);
    sem_post(&m_start_sem_);
}

void IOThread::join(){
    pthread_join(m_thread_, NULL);
}

void * IOThread::Main(void * arg){
    // arg is from IOThread this point
    IOThread * thread = static_cast<IOThread*>(arg);
    thread->m_event_loop_ = new EventLoop();
    thread->m_thread_id_ = GetThreadId();

    //wake up waiting thread
    sem_post(&thread->m_init_sem_);

    //IO thread wait starting orders
    
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id_);
    
    sem_wait(&thread->m_start_sem_);
    DEBUGLOG("IOThread %d start loop ", thread->m_thread_id_);
    thread->m_event_loop_->Loop();

    DEBUGLOG("IOThread %d end loop ", thread->m_thread_id_);
}

}
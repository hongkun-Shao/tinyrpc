#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"
#include "tinyrpc/net/eventloop.h"

#define ADD_TO_EPOLL()                                      \
    auto it = m_listen_fds.find(event->get_fd());           \
    int op = EPOLL_CTL_ADD;                                 \
    if(it != m_listen_fds.end()){                           \
        op = EPOLL_CTL_MOD;                                 \
    }                                                       \
    epoll_event temp = event->get_epoll_event();            \
    INFOLOG("epoll_event.events = %d", (int)temp.events);   \
    int res = epoll_ctl(m_epoll_fd_, op, event->get_fd(), &temp);    \
    if(res == -1){                                          \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));   \
    }                                                       \
    m_listen_fds.insert(event->get_fd());                   \
    DEBUGLOG("add event success, fd[%d]", event->get_fd()); \
    \

#define DELETE_TO_EPOLL()                                   \
    auto it = m_listen_fds.find(event->get_fd());           \
    int op = EPOLL_CTL_DEL;                                 \
    if(it == m_listen_fds.end()){                           \
        return;                                             \
    }                                                       \
    epoll_event temp = event->get_epoll_event();            \
    INFOLOG("epoll_event.events = %d", (int)temp.events);   \
    int res = epoll_ctl(m_epoll_fd_, op, event->get_fd(), &temp);    \
    if(res == -1){                                          \
        ERRORLOG("failed epoll_ctl when delete fd, errno=%d, error=%s", errno, strerror(errno));   \
    }                                                       \
    m_listen_fds.erase(event->get_fd());                    \
    DEBUGLOG("delete event success, fd[%d]", event->get_fd());      \
    \

namespace tinyrpc{
static thread_local EventLoop * t_current_eventloop = nullptr;
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events_num = 10;

EventLoop::EventLoop(){
    if(t_current_eventloop != nullptr){
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }
    m_thread_id_ = GetThreadId();

    m_epoll_fd_ = epoll_create(g_epoll_max_events_num);

    if(m_epoll_fd_ == -1){
        ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
        exit(0);
    }

    InitWakeUpFdEevent();
    InitTimer();

    INFOLOG("success create event loop in thread %d", m_thread_id_);
    t_current_eventloop = this;
}

EventLoop::~EventLoop(){
    close(m_epoll_fd_);
    if(m_wakeup_fd_){
        delete m_wakeup_fd_event_;
        m_wakeup_fd_event_ = nullptr;
    }
}

void EventLoop::Loop(){
    m_is_looping_ = true;
    while(!m_stop_flag_){
        Locker<Mutex> lock(m_mutex_);
        std::queue<std::function<void()>> temp_tasks;
        m_pending_tasks_.swap(temp_tasks);
        lock.unlock();

        while(!temp_tasks.empty()){
            std::function<void()> cb = temp_tasks.front();
            temp_tasks.pop();
            if(cb){
                cb();
            }
        }

        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events_num];
        int res = epoll_wait(m_epoll_fd_, result_events, g_epoll_max_events_num, timeout);
        DEBUGLOG("now end epoll_wait, res = %d", res);

        if(res < 0){
            ERRORLOG("epoll_wait error, errno = %d, error = %s", errno, strerror(errno));
        }else{
            for(int i = 0; i < res; ++ i){
                epoll_event trigger_event = result_events[i];
                FdEvent * fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
                if(fd_event == NULL){
                    ERRORLOG("fd_event = NULL, continue");
                    continue;
                }

                if (trigger_event.events & EPOLLIN) { 
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->get_fd())
                    AddTask(fd_event->Handler(FdEvent::IN_EVENT));
                }
                if (trigger_event.events & EPOLLOUT) { 
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->get_fd())
                    AddTask(fd_event->Handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }
}

void EventLoop::WakeUp(){
  INFOLOG("WAKE UP");
  m_wakeup_fd_event_->wakeup();
}

void EventLoop::Stop(){
    m_stop_flag_ = true;
}

void EventLoop::AddEpollEvent(FdEvent * event){
    if(IsInLoopThread()){
        ADD_TO_EPOLL();
    }else{
        auto cb = [this, event](){
            ADD_TO_EPOLL();
        };
        AddTask(cb, true);
    }
}

void EventLoop::DeleteEpollEvent(FdEvent * event){
    if (IsInLoopThread()) {
        DELETE_TO_EPOLL();
    } else {
        auto cb = [this, event]() {
        DELETE_TO_EPOLL();
    };
    AddTask(cb, true);
  }
}

bool EventLoop::IsInLoopThread(){
    return GetThreadId() == m_thread_id_;
}

void EventLoop::AddTask(std::function<void()> cb, bool is_wake_up){
    Locker<Mutex> lock(m_mutex_);
    m_pending_tasks_.push(cb);
    lock.unlock();

    if(is_wake_up){
        WakeUp();
    }
}

void EventLoop::AddTimerEvent(TimerEvent::s_ptr event){
    m_timer_->AddTimerEvent(event);
}

void EventLoop::DeleteTimerEvent(TimerEvent::s_ptr event){
    m_timer_->DeleteTimerEvent(event);
}

EventLoop* EventLoop::GetCurrentEventLoop(){
    if(t_current_eventloop){
        return t_current_eventloop;
    }
    t_current_eventloop = new EventLoop();
    return t_current_eventloop;
}

void EventLoop::DealWakeup(){
    //don't deal
}

void EventLoop::InitWakeUpFdEevent(){
    m_wakeup_fd_ = eventfd(0, EFD_NONBLOCK);
    if(m_wakeup_fd_ < 0){
        ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
        exit(0);
    }
    INFOLOG("wakeup fd = %d", m_wakeup_fd_);

    m_wakeup_fd_event_ = new WakeUpFdEvent(m_wakeup_fd_);

    m_wakeup_fd_event_->Listen(FdEvent::IN_EVENT, [this](){
        char buf[8];
        while(read(m_wakeup_fd_, buf, 8) != -1 && errno != EAGAIN){
            //read work
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd_);
    });
    AddEpollEvent(m_wakeup_fd_event_);
}

void EventLoop::InitTimer(){
    m_timer_ = new Timer();
    AddEpollEvent(m_timer_);
}

bool EventLoop::IsLooping(){
    return m_is_looping_;
}

}
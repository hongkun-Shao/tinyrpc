#ifndef TINYRPC_NET_EVENTLOOP_H
#define TINYRPC_NET_EVENTLOOP_H

#include <set>
#include <queue>
#include <pthread.h>
#include <functional>

#include "tinyrpc/tool/mutex.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/wakeup_fd_event.h"

namespace tinyrpc{

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

public:
    void Loop();

    void WakeUp();

    void Stop();

    void AddEpollEvent(FdEvent * event);

    void DeleteEpollEvent(FdEvent * event);

    bool IsInLoopThread();

    void AddTask(std::function<void()> cb, bool is_wake_up = false);

private:
    void DealWakeup();

    void InitWakeUpFdEevent();

private:
    pid_t m_thread_id_;                                     //thread where eventloop in 

    int m_epoll_fd_ {0};
    int m_wakeup_fd_ {0};

    WakeUpFdEvent * m_wakeup_fd_event_ {nullptr};

    bool m_stop_flag_ {false};                              //stop flag

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks_;     //task queue

    Mutex m_mutex_;
};

}

#endif
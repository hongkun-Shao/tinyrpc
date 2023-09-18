#ifndef TINYRPC_TOOL_FD_EVENT_H
#define TINYRPC_TOOL_FD_EVENT_H

#include <functional>
#include <sys/epoll.h>

namespace tinyrpc{

class FdEvent{
public:
    enum TriggerEvent{
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT
    };

    FdEvent(int fd);

    ~FdEvent();

    std::function<void()> Handler(TriggerEvent event_type);
    
    void Listen(TriggerEvent event_type, std::function<void()> callback);

    int get_fd() const{
        return m_fd_;
    }
    epoll_event get_epoll_event(){
        return m_listen_events_;
    }
    
protected:
    int m_fd_ {-1};
    epoll_event m_listen_events_;

    std::function<void()> m_read_callback_;
    std::function<void()> m_write_callback_;
};


}

#endif
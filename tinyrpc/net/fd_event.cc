#include <string.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/fd_event.h"

namespace tinyrpc{

FdEvent::FdEvent(int fd) : m_fd_(fd){
    memset(&m_listen_events_, 0, sizeof(m_listen_events_));
}

FdEvent::~FdEvent(){

}

std::function<void()> FdEvent::Handler(TriggerEvent event_typ){
    if(event_typ == TriggerEvent::IN_EVENT){
        return m_read_callback_;
    }else{
        return m_write_callback_;
    }
}

void FdEvent::Listen(TriggerEvent event_type, std::function<void()> callback){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events_.events |= EPOLLIN;
        m_read_callback_ = callback;
    }else{
        m_listen_events_.events |= EPOLLOUT;
        m_write_callback_ = callback;
    }

    m_listen_events_.data.ptr = this;
}

}
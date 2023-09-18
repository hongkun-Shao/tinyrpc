#ifndef TINYRPC_NET_WAKEUP_FD_EVENT_H
#define TINYRPC_NET_WAKEUP_FD_EVENT_H

#include "tinyrpc/net/fd_event.h"

namespace tinyrpc{

class WakeUpFdEvent : public FdEvent{
public:
    WakeUpFdEvent(int fd);

    ~WakeUpFdEvent();

    void wakeup();
};

}

#endif
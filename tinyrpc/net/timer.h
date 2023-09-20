#ifndef TINYRPC_NET_TIMER_H
#define TINYRPC_NET_TIMER_H

#include <map>
#include "tinyrpc/tool/mutex.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/timer_event.h"

namespace tinyrpc{

class Timer : public FdEvent{
public:
    Timer();
    ~Timer();

    void AddTimerEvent(TimerEvent::s_ptr event);

    void DeleteTimerEvent(TimerEvent::s_ptr event);

    void OnTimer();  //timer event become due, trigger callback function
private:
    //  this function is to reset timer's arrive time
    //  is different from TimerEvent::ResetArriveTime()
    void ResetArriveTime();

private:
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events_;
    Mutex m_mutex_;
};

}


#endif
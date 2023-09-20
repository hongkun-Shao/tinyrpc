#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"
#include "tinyrpc/net/timer_event.h"

namespace tinyrpc{
TimerEvent::TimerEvent(int interval, bool repeat_flag, std::function<void()> cb)
    : m_interval_(interval), m_repeat_flag_(repeat_flag), m_task_(cb){
    ResetArriveTime();    
}

void TimerEvent::ResetArriveTime(){
    m_arrive_time_ = GetNowMs() + m_interval_;
    DEBUGLOG("success create new timer event, will excute at [%lld] ms", m_arrive_time_);
}

}
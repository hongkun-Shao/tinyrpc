#ifndef TINYRPC_NET_TIMER_EVENT_H
#define TINYRPC_NET_TIMER_EVENT_H

#include <functional>
#include <memory>

namespace tinyrpc {

class TimerEvent {
 public:
  typedef std::shared_ptr<TimerEvent> s_ptr;

  TimerEvent(int interval, bool repeat_flag, std::function<void()> cb);

  int64_t get_arrive_time() const { return m_arrive_time_; }

  void set_cancle(bool value) { m_cancle_flag_ = value; }

  bool IsCancle() { return m_cancle_flag_; }

  bool IsRepeat() { return m_repeat_flag_; }

  std::function<void()> get_task() { return m_task_; }

  void ResetArriveTime();

 private:
  int64_t m_arrive_time_;  // ms
  int64_t m_interval_;     // ms
  bool m_repeat_flag_{false};
  bool m_cancle_flag_{false};

  std::function<void()> m_task_;  // task of timer event
};

}  // namespace tinyrpc

#endif
#include "tinyrpc/net/timer.h"

#include <string.h>
#include <sys/timerfd.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"

namespace tinyrpc {

Timer::Timer() {
  m_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  DEBUGLOG("timer fd = %d", m_fd_);

  // Placing fd EPOLLIN events on eventloop for listening
  Listen(FdEvent::IN_EVENT, std::bind(&Timer::OnTimer, this));
}

Timer::~Timer() {}

void Timer::AddTimerEvent(TimerEvent::s_ptr event) {
  bool reset_flag = false;

  Locker<Mutex> lock(m_mutex_);
  if (m_pending_events_.empty()) {
    reset_flag = true;
  } else {
    auto it = m_pending_events_.begin();
    if (it->second->get_arrive_time() > event->get_arrive_time()) {
      reset_flag = true;
    }
  }
  m_pending_events_.emplace(event->get_arrive_time(), event);
  lock.unlock();

  if (reset_flag) {
    ResetArriveTime();
  }

  DEBUGLOG("success add TimerEvent whose arrive time %lld ms",
           event->get_arrive_time());
}

void Timer::DeleteTimerEvent(TimerEvent::s_ptr event) {
  event->set_cancle(true);

  Locker<Mutex> lock(m_mutex_);
  auto st = m_pending_events_.begin();
  auto ed = m_pending_events_.end();

  auto it = st;
  for (it = st; it != ed; ++it) {
    if (it->second == event) {
      break;
    }
  }

  if (it != ed) {
    m_pending_events_.erase(it);
  }
  lock.unlock();
  DEBUGLOG("success delete TimerEvent whose arrive time %lld ms",
           event->get_arrive_time());
}

void Timer::OnTimer() {
  // Handle buffer data to prevent continuous triggering of readable events
  DEBUGLOG("OnTimer");
  char buf[128];
  while (1) {
    if (read(m_fd_, buf, 128) == -1 && errno == EAGAIN) {
      break;
    }
  }

  // excute timer tasks
  int64_t now = GetNowMs();

  std::vector<TimerEvent::s_ptr> sptemp;
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;

  Locker<Mutex> lock(m_mutex_);
  auto it = m_pending_events_.begin();
  for (it = m_pending_events_.begin(); it != m_pending_events_.end(); ++it) {
    if (it->first <= now) {
      sptemp.push_back(it->second);
      tasks.push_back({it->second->get_arrive_time(), it->second->get_task()});
    } else {
      break;
    }
  }

  m_pending_events_.erase(m_pending_events_.begin(), it);
  lock.unlock();

  // add the timer event whose repeat_flag is true to Timer
  for (auto i = sptemp.begin(); i != sptemp.end(); ++i) {
    if ((*i)->IsRepeat()) {
      // adjust arrivetime
      (*i)->ResetArriveTime();
      AddTimerEvent(*i);
    }
  }

  ResetArriveTime();

  for (auto i : tasks) {
    if (i.second) {
      i.second();
    }
  }
}

void Timer::ResetArriveTime() {
  Locker<Mutex> lock(m_mutex_);
  auto temp = m_pending_events_;
  lock.unlock();

  if (temp.size() == 0) {
    return;
  }

  int64_t now = GetNowMs();
  auto it = temp.begin();
  int64_t interval = 0;
  if (it->second->get_arrive_time() > now) {
    interval = it->second->get_arrive_time() - now;
  } else {
    interval = 100;
  }

  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = interval / 1000;
  ts.tv_nsec = (interval % 1000) * 1000000;  // 1 ms = 1000000 ns

  itimerspec value;
  memset(&value, 0, sizeof(value));
  value.it_value = ts;

  int res = timerfd_settime(m_fd_, 0, &value, NULL);
  if (res != 0) {
    ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno,
             strerror(errno));
  }
  DEBUGLOG("timer reset to %lld", now + interval);
}

}  // namespace tinyrpc

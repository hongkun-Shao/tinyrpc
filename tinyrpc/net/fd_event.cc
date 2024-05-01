#include "tinyrpc/net/fd_event.h"

#include <fcntl.h>
#include <string.h>

#include "tinyrpc/tool/log.h"

namespace tinyrpc {

FdEvent::FdEvent(int fd) : m_fd_(fd) {
  memset(&m_listen_events_, 0, sizeof(m_listen_events_));
}

FdEvent::FdEvent() { memset(&m_listen_events_, 0, sizeof(m_listen_events_)); }

FdEvent::~FdEvent() {}

void FdEvent::SetNonBlock() {
  int flag = fcntl(m_fd_, F_GETFL, 0);
  if (flag & O_NONBLOCK) {
    return;
  }
  fcntl(m_fd_, F_SETFL, flag | O_NONBLOCK);
}

std::function<void()> FdEvent::Handler(TriggerEvent event_type) {
  if (event_type == TriggerEvent::IN_EVENT) {
    return m_read_callback_;
  } else if (event_type == TriggerEvent::OUT_EVENT) {
    return m_write_callback_;
  } else if (event_type == TriggerEvent::ERROR_EVENT) {
    return m_error_callback_;
  }
  return nullptr;
}

void FdEvent::Listen(TriggerEvent event_type, std::function<void()> callback,
                     std::function<void()> error_callback /*= nullptr*/) {
  if (event_type == TriggerEvent::IN_EVENT) {
    m_listen_events_.events |= EPOLLIN;
    m_read_callback_ = callback;
  } else {
    m_listen_events_.events |= EPOLLOUT;
    m_write_callback_ = callback;
  }

  if (m_error_callback_ == nullptr) {
    m_error_callback_ = error_callback;
  } else {
    m_error_callback_ = nullptr;
  }

  m_listen_events_.data.ptr = this;
}

void FdEvent::Cancle(TriggerEvent event_type) {
  if (event_type == TriggerEvent::IN_EVENT) {
    m_listen_events_.events &= (~EPOLLIN);
  } else {
    m_listen_events_.events &= (~EPOLLOUT);
  }
}

}  // namespace tinyrpc
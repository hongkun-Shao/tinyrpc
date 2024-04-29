#ifndef TINYRPC_TOOL_FD_EVENT_H
#define TINYRPC_TOOL_FD_EVENT_H

#include <sys/epoll.h>

#include <functional>

namespace tinyrpc {

class FdEvent {
 public:
  enum TriggerEvent {
    IN_EVENT = EPOLLIN,
    OUT_EVENT = EPOLLOUT,
    ERROR_EVENT = EPOLLERR,
  };

  FdEvent(int fd);

  FdEvent();

  virtual ~FdEvent();

  void SetNonBlock();

  std::function<void()> Handler(TriggerEvent event_type);

  void Listen(TriggerEvent event_type, std::function<void()> callback,
              std::function<void()> error_callback = nullptr);

  // cancle listen
  void Cancle(TriggerEvent event_type);

  int get_fd() const { return m_fd_; }
  epoll_event get_epoll_event() { return m_listen_events_; }

 protected:
  int m_fd_{-1};
  epoll_event m_listen_events_;

  std::function<void()> m_read_callback_{nullptr};
  std::function<void()> m_write_callback_{nullptr};
  std::function<void()> m_error_callback_{nullptr};
};

}  // namespace tinyrpc

#endif
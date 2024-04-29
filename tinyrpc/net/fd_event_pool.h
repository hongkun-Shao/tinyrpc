#ifndef TINYRPC_NET_FD_EVENT_POOL_H
#define TINYRPC_NET_FD_EVENT_POOL_H

#include <vector>

#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/tool/mutex.h"

namespace tinyrpc {

class FdEventPool {
 public:
  FdEventPool(int size);

  ~FdEventPool();

  FdEvent* get_fd_event(int fd);

 public:
  static FdEventPool* get_fd_event_pool();

 private:
  int m_size_{0};
  std::vector<FdEvent*> m_fd_pool_;
  Mutex m_mutex_;
};

}  // namespace tinyrpc

#endif
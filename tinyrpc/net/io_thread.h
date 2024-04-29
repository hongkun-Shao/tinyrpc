#ifndef TINYRPC_NET_IO_THREAD_H
#define TINYRPC_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>

#include "tinyrpc/net/eventloop.h"

namespace tinyrpc {

class IOThread {
 public:
  IOThread();
  ~IOThread();

  EventLoop* get_eventloop();

  void start();

  void join();

 public:
  static void* Main(void* arg);

 private:
  pid_t m_thread_id_{-1};
  pthread_t m_thread_{0};  // thread handle

  EventLoop* m_event_loop_{nullptr};  // current thread's eventloop

  sem_t m_init_sem_;
  sem_t m_start_sem_;
};

}  // namespace tinyrpc
#endif
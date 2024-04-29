#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include <google/protobuf/service.h>

#include <set>

#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/io_thread_pool.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_acceptor.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/tool/zookeeper_util.h"

namespace tinyrpc {

class TcpServer {
 public:
  typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;

  TcpServer(NetAddr::s_ptr local_addr);

  ~TcpServer();

  //向 注册中心（zookeeper） 注册能够提供的服务
  void RegisterServiceToCenter(service_s_ptr service);

  void Start();

 private:
  void Init();

  // excute when new client connect success
  void OnAccept();

 private:
  TcpAcceptor::s_ptr m_acceptor_;

  NetAddr::s_ptr m_local_addr_;

  EventLoop* m_main_event_loop_{nullptr};  // main Reactor

  IOThreadPool* m_io_thread_pool_{nullptr};  // subReactor 组

  FdEvent* m_listen_fd_event_;

  int m_client_counts_{0};

  std::set<TcpConnection::s_ptr> m_client_;

  ZkClient* m_zookeeper_client_{nullptr};
};

}  // namespace tinyrpc

#endif
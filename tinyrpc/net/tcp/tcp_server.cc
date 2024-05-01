#include "tinyrpc/net/tcp/tcp_server.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/rpc/rpc_dispatcher.h"
#include "tinyrpc/tool/config.h"
#include "tinyrpc/tool/log.h"

namespace tinyrpc {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr_(local_addr) {
  Init();

  INFOLOG("tinyrpc TcpServer listen sucess on [%s]",
          m_local_addr_->ToString().c_str());
}

TcpServer::~TcpServer() {
  if (m_main_event_loop_) {
    delete m_main_event_loop_;
    m_main_event_loop_ = nullptr;
  }
  if (m_zookeeper_client_) {
    delete m_zookeeper_client_;
    m_zookeeper_client_ = nullptr;
  }
}

void TcpServer::RegisterServiceToCenter(service_s_ptr service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_zookeeper_client_->Start();
  std::string service_path = "/" + service_name;
  char service_path_data[128] = {0};
  sprintf(service_path_data, "%s", m_local_addr_->ToString().c_str());
  m_zookeeper_client_->Create(service_path.c_str(), service_path_data,
                              strlen(service_path_data), 0);
  INFOLOG("netaddr: %s", m_local_addr_->ToString().c_str());
  INFOLOG("service: %s has registered to Center, path: %s, data: %s",
          service_name.c_str(), service_path.c_str(), service_path_data);
}

void TcpServer::Start() {
  m_io_thread_pool_->Start();
  m_main_event_loop_->Loop();
}

void TcpServer::Init() {
  m_acceptor_ = std::make_shared<TcpAcceptor>(m_local_addr_);
  m_main_event_loop_ = EventLoop::GetCurrentEventLoop();
  m_io_thread_pool_ = new IOThreadPool(Config::GetGlobalConfig()->m_io_threads);

  m_listen_fd_event_ = new FdEvent(m_acceptor_->get_listenfd());
  m_listen_fd_event_->Listen(FdEvent::IN_EVENT,
                             std::bind(&TcpServer::OnAccept, this));
  m_main_event_loop_->AddEpollEvent(m_listen_fd_event_);

  m_zookeeper_client_ = new ZkClient();
}

void TcpServer::OnAccept() {
  auto re = m_acceptor_->Accept();
  int client_fd = re.first;
  NetAddr::s_ptr peer_addr = re.second;

  m_client_counts_++;

  // 把 cleintfd 添加到任意 IO 线程里面
  IOThread* io_thread = m_io_thread_pool_->get_iothread();
  TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(
      io_thread->get_eventloop(), client_fd, 128, peer_addr, m_local_addr_);
  connetion->set_state(Connected);

  m_client_.insert(connetion);

  INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

}  // namespace tinyrpc

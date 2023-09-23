#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/tcp_server.h"

namespace tinyrpc{

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr_(local_addr){
    Init();

    INFOLOG("tinyrpc TcpServer listen sucess on [%s]", m_local_addr_->ToString().c_str());
}

TcpServer::~TcpServer(){
    if (m_main_event_loop_) {
        delete m_main_event_loop_;
        m_main_event_loop_ = nullptr;
    }
}

void TcpServer::Start(){
    m_io_thread_pool_->Start();
    m_main_event_loop_->Loop();
}

void TcpServer::Init(){
    m_acceptor_ = std::make_shared<TcpAcceptor>(m_local_addr_);
    m_main_event_loop_ = EventLoop::GetCurrentEventLoop();
    m_io_thread_pool_ = new IOThreadPool(2);

    m_listen_fd_event_ = new FdEvent(m_acceptor_->get_listenfd());
    m_listen_fd_event_->Listen(FdEvent::IN_EVENT, std::bind(&TcpServer::OnAccept, this));
    m_main_event_loop_->AddEpollEvent(m_listen_fd_event_);
}

void TcpServer::OnAccept(){
    int client_fd = m_acceptor_->Accept();
    //FdEvent client_fd_event(client_fd);
    m_client_counts_ ++;
    // TODO: add cleintfd to any IO thread
    // m_io_thread_pool_->GetIOThread()->GetEventLoop()->AddEpollEvent(client_fd_event);
    
    INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

}

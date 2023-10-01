#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/fd_event_pool.h"

namespace tinyrpc{

TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr_(peer_addr){
    m_event_loop_ = EventLoop::GetCurrentEventLoop();
    m_fd_ = socket(peer_addr->GetFamily(), SOCK_STREAM, 0);
    if(m_fd_ < 0){
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }

    m_fd_event_ = FdEventPool::get_fd_event_pool()->get_fd_event(m_fd_);
    m_fd_event_->SetNonBlock();

    m_connection_ = std::make_shared<TcpConnection>(m_event_loop_, m_fd_, 128, peer_addr);
    m_connection_->set_connection_type(TcpConnectionByClient);  
}

TcpClient::~TcpClient(){
    DEBUGLOG("TcpClient::~TcpClient()");
    if (m_fd_ > 0) {
        close(m_fd_);
    }
}


void TcpClient::Connect(std::function<void()> done){
    int res = connect(m_fd_, m_peer_addr_->GetSockAddr(), m_peer_addr_->GetSockLen());
    if(res == 0){
        DEBUGLOG("connect [%s] sussess", m_peer_addr_->ToString().c_str());
        if (done) {
            done();
        }
    }else if(res == -1){
        if(errno == EINPROGRESS){
            //epoll 监听可写事件，然后判断错误码
            m_fd_event_->Listen(FdEvent::OUT_EVENT, [this, done](){
                int error = 0;
                socklen_t error_len = sizeof(error);
                getsockopt(m_fd_, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if(error == 0){
                    DEBUGLOG("connect [%s] success", m_peer_addr_->ToString().c_str());
                    if(done){
                        done();
                    }
                }else{
                    ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));    
                }
                m_fd_event_->Cancle(FdEvent::OUT_EVENT);
                m_event_loop_->AddEpollEvent(m_fd_event_);
            });
            m_event_loop_->AddEpollEvent(m_fd_event_);
            if(!m_event_loop_->IsLooping()){
                m_event_loop_->Loop();
            }
        }else{
            ERRORLOG("connect errror, errno = %d, error = %s", errno, strerror(errno));
        }
    }
}

void TcpClient::WriteMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){

}


void TcpClient::ReadMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){
    
}

}
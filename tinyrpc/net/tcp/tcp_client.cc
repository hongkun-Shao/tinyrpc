#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/fd_event_pool.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/tool/error_code.h"
#include "tinyrpc/net/tcp/net_addr.h"

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

    m_connection_ = std::make_shared<TcpConnection>(m_event_loop_, m_fd_, 128, peer_addr, nullptr, TcpConnectionByClient);
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
        m_connection_->set_state(Connected);
        initLocalAddr();
        if (done) {
            done();
        }
    }else if(res == -1){
        if(errno == EINPROGRESS){
            //epoll 监听可写事件，然后判断错误码
            m_fd_event_->Listen(FdEvent::OUT_EVENT, [this, done](){
                int rt = ::connect(m_fd_, m_peer_addr_->GetSockAddr(), m_peer_addr_->GetSockLen());
                if((rt < 0 && errno == EISCONN) || (rt == 0)){
                    DEBUGLOG("connect [%s] sussess", m_peer_addr_->ToString().c_str());
                    initLocalAddr();
                    m_connection_->set_state(Connected);
                }else{
                    if(errno == ECONNREFUSED){
                        m_connect_error_code_ = ERROR_PEER_CLOSED;
                        m_connect_error_info_ = "connect refused, sys error = " + std::string(strerror(errno));
                    }else{
                        m_connect_error_code_ = ERROR_FAILED_CONNECT;
                        m_connect_error_info_ = "connect unkonwn error, sys error = " + std::string(strerror(errno));
                    }
                    ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
                    close(m_fd_);
                    m_fd_ = socket(m_peer_addr_->GetFamily(), SOCK_STREAM, 0);
                }

                // 连接完后需要去掉可写事件的监听，不然会一直触发
                m_event_loop_->DeleteEpollEvent(m_fd_event_);
                DEBUGLOG("now begin to done");
                // 如果连接完成，才会执行回调函数
                if (done) {
                    done();
                }
            });

            m_event_loop_->AddEpollEvent(m_fd_event_);
            if(!m_event_loop_->IsLooping()){
                m_event_loop_->Loop();
            }
        }else{
            ERRORLOG("connect errror, errno = %d, error = %s", errno, strerror(errno));
            m_connect_error_code_ = ERROR_FAILED_CONNECT;
            m_connect_error_info_ = "connect error, sys error = " + std::string(strerror(errno));
            if(done){
                done();
            }
        }
    }
}

void TcpClient::WriteMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){
    // write message and done into buffer
    // start listen write
    m_connection_->PushSendMessage(message, done);
    m_connection_->ListenWrite();
}


void TcpClient::ReadMessage(const std::string & msg_id, std::function<void(AbstractProtocol::s_ptr)> done){
    // start listen read
    // decode buffer data to get message object, check msg_id equal msg_id, if equal, do callback
    m_connection_->PushReadMessage(msg_id, done);
    m_connection_->ListenRead();
}

void TcpClient::stop() {
    if(m_event_loop_->IsLooping()){
        m_event_loop_->Stop();
    }
}

int TcpClient::getConnectErrorCode() {
    return m_connect_error_code_;
}

std::string TcpClient::getConnectErrorInfo() {
    return m_connect_error_info_;
}

NetAddr::s_ptr TcpClient::getPeerAddr() {
    return m_peer_addr_;
}

NetAddr::s_ptr TcpClient::getLocalAddr() {
      return m_local_addr_;
}

void TcpClient::initLocalAddr() {
    sockaddr_in local_addr;
    socklen_t len = sizeof(local_addr);

    int ret = getsockname(m_fd_, reinterpret_cast<sockaddr*>(&local_addr), &len);
    if (ret != 0) {
        ERRORLOG("initLocalAddr error, getsockname error. errno=%d, error=%s", errno, strerror(errno));
        return;
    }

    m_local_addr_ = std::make_shared<IPNetAddr>(local_addr);
}

void TcpClient::addTimerEvent(TimerEvent::s_ptr timer_event) {
    m_event_loop_->AddTimerEvent(timer_event);
}

}
#include <unistd.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/fd_event_pool.h"
#include "tinyrpc/net/tcp/tcp_connection.h"

namespace tinyrpc{
    
TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    : m_event_loop_(event_loop), m_peer_addr_(peer_addr), m_state_(NotConnected), m_fd_(fd){
    m_in_buffer_ = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer_ = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event_ = FdEventPool::get_fd_event_pool()->get_fd_event(fd);
    m_fd_event_->SetNonBlock();
    m_fd_event_->Listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::OnRead, this));
    m_event_loop_->AddEpollEvent(m_fd_event_);

}

TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection");
}

void TcpConnection::OnRead(){
    //put the read message  into in_buffer
    if(m_state_ != Connected){
        ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr_->ToString().c_str(), m_fd_);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;

    while(!is_read_all){
        if(m_in_buffer_->GetWriteSize() == 0){
            m_in_buffer_->ResizeBuffer(2 * m_in_buffer_->m_buffer_.size());
        }
        int read_count = m_in_buffer_->GetWriteSize();
        int write_index = m_in_buffer_->get_write_index();

        int res = read(m_fd_, &(m_in_buffer_->m_buffer_[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", res, m_peer_addr_->ToString().c_str(), m_fd_);    
        
        if(res > 0){
            m_in_buffer_->MoveWriteIndex(res);
            if(res == read_count){
                continue;
            }else if(res < read_count){
                is_read_all = true;
                break;
            }
        }else if(res == 0){
            is_close = true;
            break;
        }else if(res == -1 && errno == EAGAIN){
            is_read_all = true;
            break;
        }
    }

    if(is_close){
        //TODO: 
        INFOLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr_->ToString().c_str(), m_fd_);
        clear();
        return;
    }
    if (!is_read_all) {
        ERRORLOG("not read all data");
    }

    // TODO: 简单的 echo, 后面补充 RPC 协议解析 
    Excute();
}

void TcpConnection::Excute(){
    //将 RPC 请求执行业务逻辑，获取 RPC 响应, 再把 RPC 响应发送回去
    std::vector<char> temp;
    int size = m_in_buffer_->GetReadSize();
    temp.resize(size);
    m_in_buffer_->ReadFromBuffer(temp, size);

    std::string msg;
    for(size_t i = 0; i < temp.size(); ++ i){
        msg += temp[i];
    }    

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr_->ToString().c_str());
    m_out_buffer_->WriteToBuffer(msg.c_str(), msg.length());
    m_fd_event_->Listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::OnWrite, this));
    m_event_loop_->AddEpollEvent(m_fd_event_);
}

void TcpConnection::OnWrite(){
    //将当前out_buffer里面的数据全部发送给client
    if(m_state_ != Connected){
        ERRORLOG("OnWrite error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr_->ToString().c_str(), m_fd_);
        return;
    }

    bool is_write_all = false;
    while(true){
        if(m_out_buffer_->GetReadSize() == 0){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr_->ToString().c_str());
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer_->GetReadSize();
        int read_index = m_out_buffer_->get_read_index();

        int res = write(m_fd_, &(m_out_buffer_->m_buffer_[read_index]), write_size);
        if(res >= write_size){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr_->ToString().c_str());
            is_write_all = true;
            break;
        }else if(res == -1 && errno == EAGAIN){
            // 发送缓冲区已满，不能再发送了。
            // 这种情况我们等下次 fd 可写的时候再次发送数据即可
            ERRORLOG("write data error, errno==EAGIN and rt == -1");
            break;
        }
    }
    if(is_write_all){
        m_fd_event_->Cancle(FdEvent::OUT_EVENT);
        m_event_loop_->AddEpollEvent(m_fd_event_);
    }
}

void TcpConnection::set_state(const TcpState state){
    m_state_ = Connected;
}

TcpState TcpConnection::get_state(){
    return m_state_;
}

void TcpConnection::clear(){
    // 处理一些关闭连接后的清理动作
    if(m_state_ = Closed){
        return;
    }
    m_fd_event_->Cancle(FdEvent::IN_EVENT);
    m_fd_event_->Cancle(FdEvent::OUT_EVENT);
    m_event_loop_->DeleteEpollEvent(m_fd_event_);
    m_state_ = Closed;
}

// 服务器主动关闭连接
void TcpConnection::shutdown(){
    if(m_state_ == Closed || m_state_ == NotConnected){
        return;
    }
    //处于半关闭
    m_state_ = HalfClosing;

    // 调用 shutdown 关闭读写，意味着服务器不会再对这个 fd 进行读写操作了
    // 发送 FIN 报文， 触发了四次挥手的第一个阶段
    // 当 fd 发生可读事件，但是可读的数据为0，即 对端发送了 FIN
    ::shutdown(m_fd_, SHUT_RDWR);
}

void TcpConnection::set_connection_type(TcpConnectionType type){
    m_connection_type_ = type;
}

}
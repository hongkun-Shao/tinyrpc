#include <unistd.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/fd_event_pool.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/coder/string_coder.h"
#include "tinyrpc/net/coder/tinypb_coder.h"
namespace tinyrpc{
    
TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type /* = TcpConnectionByServer*/)
                            : m_event_loop_(event_loop), m_peer_addr_(peer_addr), m_local_addr_(local_addr), m_state_(NotConnected), m_fd_(fd), m_connection_type_(type){
    m_in_buffer_ = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer_ = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event_ = FdEventPool::get_fd_event_pool()->get_fd_event(fd);
    m_fd_event_->SetNonBlock();
    
    m_coder_ = new TinyPBCoder();

    if(m_connection_type_ == TcpConnectionByServer){
        ListenRead();
    }
}

TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection");
    if(m_coder_){
        delete m_coder_;
        m_coder_ = nullptr;
    }
}


void TcpConnection::OnRead() {
    // 1. 从 socket 缓冲区，调用 系统的 read 函数读取字节 in_buffer 里面

    if (m_state_ != Connected) {
        ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr_->ToString().c_str(), m_fd_);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while(!is_read_all) {
        if (m_in_buffer_->GetWriteSize() == 0) {
            m_in_buffer_->ResizeBuffer(2 * m_in_buffer_->m_buffer_.size());
        }
        int read_count = m_in_buffer_->GetWriteSize();
        int write_index = m_in_buffer_->get_write_index(); 

        int rt = read(m_fd_, &(m_in_buffer_->m_buffer_[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr_->ToString().c_str(), m_fd_);
        if (rt > 0) {
            m_in_buffer_->MoveWriteIndex(rt);
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            }
        } else if (rt == 0) {
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
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
    if(m_connection_type_ == TcpConnectionByServer){
        //将 RPC 请求执行业务逻辑，获取 RPC 响应, 再把 RPC 响应发送回去
        std::vector<AbstractProtocol::s_ptr> result;
        std::vector<AbstractProtocol::s_ptr> replay_messages;
        m_coder_->Decode(result, m_in_buffer_);
        for (size_t i = 0;  i < result.size(); ++i) {
            // 1. 针对每一个请求，调用 rpc 方法，获取响应 message
            // 2. 将响应 message 放入到发送缓冲区，监听可写事件回包
            INFOLOG("success get request[%s] from client[%s]", result[i]->m_msg_id_.c_str(), m_peer_addr_->ToString().c_str());

            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            //message->m_pb_data_ = "hello. this is tinyrpc test data";
            //message->m_msg_id_ = result[i]->m_msg_id_;
            RpcDispatcher::GetRpcDispatcher()->dispatch(result[i], message, this);
            replay_messages.emplace_back(message);
        }

        m_coder_->Encode(replay_messages, m_out_buffer_);
        ListenWrite();
    }else{
        //decode the char stream in buffer, get the message object, and excute the callback done
        std::vector<AbstractProtocol::s_ptr> res;
        m_coder_->Decode(res, m_in_buffer_);

        for(size_t i = 0; i < res.size(); ++ i){
            std::string msg_id = res[i]->m_msg_id_;
            auto it = m_read_dones_.find(msg_id);
            if(it != m_read_dones_.end()){
                it->second(res[i]);
            }
        }
    }
}

void TcpConnection::OnWrite(){
    //send all current out_buffer data to client
    if(m_state_ != Connected){
        ERRORLOG("OnWrite error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr_->ToString().c_str(), m_fd_);
        return;
    }

    if(m_connection_type_ == TcpConnectionByClient){
        // encode message to char stream
        // put char stream to buffer, and send all

        std::vector<AbstractProtocol::s_ptr> message;
        for(size_t i = 0; i < m_write_dones_.size(); ++ i){
            message.push_back(m_write_dones_[i].first);
        }

        m_coder_->Encode(message, m_out_buffer_);
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

    if(m_connection_type_ == TcpConnectionByClient){
        for(size_t i = 0; i < m_write_dones_.size(); ++ i){
            m_write_dones_[i].second(m_write_dones_[i].first);
        }
        m_write_dones_.clear();
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
    if(m_state_ == Closed){
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

void TcpConnection::ListenWrite(){

    m_fd_event_->Listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::OnWrite, this));
    m_event_loop_->AddEpollEvent(m_fd_event_);
}


void TcpConnection::ListenRead(){
    m_fd_event_->Listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::OnRead, this));
    m_event_loop_->AddEpollEvent(m_fd_event_);
}


void TcpConnection::PushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_write_dones_.push_back(std::make_pair(message, done));
}

void TcpConnection::PushReadMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_read_dones_.insert(std::make_pair(msg_id, done));
}

}
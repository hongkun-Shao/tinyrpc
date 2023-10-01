#ifndef TINYRPC_NET_TCP_TCP_CONNECTION_H
#define TINYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include "tinyrpc/net/io_thread.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace tinyrpc{
enum TcpState{
    NotConnected = 1,
    Connected,
    HalfClosing,
    Closed,
};

enum TcpConnectionType{
    TcpConnectionByServer = 1,  
    TcpConnectionByClient, 
};

class TcpConnection{
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;

public:
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

    ~TcpConnection();

    void OnRead();

    void Excute();

    void OnWrite();

    void set_state(const TcpState state);

    TcpState get_state();

    void clear();

    // 服务器主动关闭连接
    void shutdown();

    void set_connection_type(TcpConnectionType type);
private:
    EventLoop* m_event_loop_ {nullptr};   //the io thread has the connection

    NetAddr::s_ptr m_local_addr_;
    NetAddr::s_ptr m_peer_addr_;

    TcpBuffer::s_ptr m_in_buffer_;   // recv buffer
    TcpBuffer::s_ptr m_out_buffer_;  // send buffer


    FdEvent* m_fd_event_ {nullptr};

    TcpState m_state_;

    int m_fd_ {0};

    TcpConnectionType m_connection_type_ {TcpConnectionByServer};
    
};

}

#endif
#ifndef TINYRPC_NET_TCP_TCP_CLIENT_H
#define TINYRPC_NET_TCP_TCP_CLIENT_H

#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/abstract_protocol.h"

namespace tinyrpc{

class TcpClient{
public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    // 异步的进行 conenct
    // 如果connect 成功，done 会被执行
    void Connect(std::function<void()> done);

    // 异步的发送 message
    // 如果发送 message 成功，会调用 done 函数， 函数的入参就是 message 对象 
    void WriteMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);


    // 异步的读取 message
    // 如果读取 message 成功，会调用 done 函数， 函数的入参就是 message 对象 
    void ReadMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
private:
    NetAddr::s_ptr m_peer_addr_;
    EventLoop * m_event_loop_ {nullptr};

    int m_fd_ {-1};
    FdEvent * m_fd_event_ {nullptr};

    TcpConnection::s_ptr m_connection_;
};

}

#endif
#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H


#include "tinyrpc/net/tcp/tcp_acceptor.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/io_thread_pool.h"

namespace tinyrpc{

class TcpServer{
public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();

    void Start();

private:
    void Init();

    //excute when new client connect success
    void OnAccept();

private:
    TcpAcceptor::s_ptr m_acceptor_;

    NetAddr::s_ptr m_local_addr_;
    
    EventLoop * m_main_event_loop_ {nullptr}; //main Reactor

    IOThreadPool * m_io_thread_pool_ {nullptr}; //subReactor 组
    
    FdEvent * m_listen_fd_event_;

    int m_client_counts_ {0};
};

}

#endif
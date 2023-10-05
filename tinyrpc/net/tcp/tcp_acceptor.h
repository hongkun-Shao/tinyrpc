#ifndef TINYRPC_NET_TCP_TCP_ACCEPTOR_H
#define TINYRPC_NET_TCP_TCP_ACCEPTOR_H

#include <memory>
#include "tinyrpc/net/tcp/net_addr.h"

namespace tinyrpc{

class TcpAcceptor{
public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;

    TcpAcceptor(NetAddr::s_ptr local_addr);

    ~TcpAcceptor();

    std::pair<int, NetAddr::s_ptr> Accept();

    int get_listenfd();
private:
    NetAddr::s_ptr m_local_addr_; //listen address ip:port
    
    int m_family_ {-1};
    int m_listenfd_ {-1};

};

}

#endif
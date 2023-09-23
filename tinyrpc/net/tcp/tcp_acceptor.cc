
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_acceptor.h"

namespace tinyrpc{

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) : m_local_addr_(local_addr){
    if(!local_addr->CheckVaild()){
        ERRORLOG("invaild local addr %s", local_addr->ToString().c_str());
        exit(0);
    }

    m_family_ = m_local_addr_->GetFamily();
    m_listenfd_ = socket(m_family_, SOCK_STREAM, 0);

    if (m_listenfd_ < 0) {
        ERRORLOG("invalid listenfd %d", m_listenfd_);
        exit(0);
    }

    int val = 1;
    if (setsockopt(m_listenfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
    }

    socklen_t len = m_local_addr_->GetSockLen();
    if(bind(m_listenfd_, m_local_addr_->GetSockAddr(), len) != 0) {
        ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }
    if(listen(m_listenfd_, 1000) != 0) {
        ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }

}   

TcpAcceptor::~TcpAcceptor(){
    
}

int TcpAcceptor::Accept(){
    if(m_family_ == AF_INET){
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        
        int client_fd = accept(m_listenfd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if(client_fd < 0){
            ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));
        }
        
        IPNetAddr peer_addr(client_addr);
        INFOLOG("A client have accpeted succ, peer addr [%s]", peer_addr.ToString().c_str());
        return client_fd;
    }else{
        return -1;
    }
}

int TcpAcceptor::get_listenfd(){
    return m_listenfd_;
}

}
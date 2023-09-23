#ifndef TINYRPC_NET_TCP_NET_ADDR_H
#define TINYRPC_NET_TCP_NET_ADDR_H

#include <memory>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace tinyrpc{

class NetAddr{
public:
    typedef std::shared_ptr<NetAddr> s_ptr;

    virtual sockaddr * GetSockAddr() = 0;

    virtual socklen_t GetSockLen() = 0;

    virtual int GetFamily() = 0;

    virtual std::string ToString() = 0;

    virtual bool CheckVaild() = 0;
};

class IPNetAddr : public NetAddr{
public:
    IPNetAddr(const std::string& ip, uint16_t port);
  
    IPNetAddr(const std::string& addr);

    IPNetAddr(sockaddr_in addr);

    sockaddr* GetSockAddr();

    socklen_t GetSockLen();

    int GetFamily();

    std::string ToString();

    bool CheckVaild();
    
private:
    std::string m_ip_;
    uint16_t m_port_ {0};

    sockaddr_in m_addr_;
};

}

#endif
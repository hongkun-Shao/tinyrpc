#include <string.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/tcp/net_addr.h"

namespace tinyrpc{

IPNetAddr::IPNetAddr(const std::string& ip, uint16_t port) : m_ip_(ip), m_port_(port){
    memset(&m_addr_, 0, sizeof(m_addr_));
    m_addr_.sin_family = AF_INET;
    m_addr_.sin_addr.s_addr = inet_addr(m_ip_.c_str());
    m_addr_.sin_port = htons(m_port_);
}

IPNetAddr::IPNetAddr(const std::string& addr){
    size_t i = addr.find_first_of(":");
    if (i == addr.npos) {
        ERRORLOG("invalid ipv4 addr %s", addr.c_str());
        return;
    }
    m_ip_ = addr.substr(0, i);
    m_port_ = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());

    memset(&m_addr_, 0, sizeof(m_addr_));
    m_addr_.sin_family = AF_INET;
    m_addr_.sin_addr.s_addr = inet_addr(m_ip_.c_str());
    m_addr_.sin_port = htons(m_port_);

}

IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr_(addr) {
  m_ip_ = std::string(inet_ntoa(m_addr_.sin_addr));
  m_port_ = ntohs(m_addr_.sin_port);
}

sockaddr* IPNetAddr::GetSockAddr(){
    return reinterpret_cast<sockaddr*>(&m_addr_);
}

socklen_t IPNetAddr::GetSockLen(){
    return sizeof(m_addr_);
}

int IPNetAddr::GetFamily(){
    return AF_INET;
}

std::string IPNetAddr::ToString(){
    std::string res;
    res = m_ip_ + ":" + std::to_string(m_port_);
    return res;
}

bool IPNetAddr::CheckVaild(){
    if(m_ip_.empty()){
        return false;
    }
    if(m_port_ < 0 || m_port_ > 65536){
        return false;
    }

    //check ip is vaild
    if(inet_addr(m_ip_.c_str()) == INADDR_NONE){
        return false;
    }
    return true;
}

}
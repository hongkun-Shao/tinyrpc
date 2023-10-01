#include <memory>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_server.h"

void test_tcp_server(){
    tinyrpc::IPNetAddr::s_ptr addr = std::make_shared<tinyrpc::IPNetAddr>("127.0.0.1", 1731);
    DEBUGLOG("create addr %s", addr->ToString().c_str());

    tinyrpc::TcpServer tcp_server(addr);

    tcp_server.Start();
}

int main() {

  tinyrpc::Config::SetGlobalConfig("../conf/tinyrpc.xml");
  tinyrpc::Logger::InitGlobalLogger();

  test_tcp_server();
  
}
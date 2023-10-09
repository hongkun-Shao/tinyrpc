#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <google/protobuf/service.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/config.h"
#include "tinyrpc/tool/log.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/coder/string_coder.h"
#include "tinyrpc/net/coder/abstract_protocol.h"
#include "tinyrpc/net/coder/tinypb_coder.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/rpc/rpc_dispatcher.h"

#include "order.pb.h"

class OrderImpl : public Order {
 public:
  void makeOrder(google::protobuf::RpcController* controller,
                      const ::makeOrderRequest* request,
                      ::makeOrderResponse* response,
                      ::google::protobuf::Closure* done) {
    APPDEBUGLOG("start sleep 5s");
    sleep(5);
    APPDEBUGLOG("end sleep 5s");
    
    if (request->price() < 10) {
      response->set_ret_code(-1);
      response->set_res_info("short balance");
      return;
    }
    response->set_order_id("20230514");
    APPDEBUGLOG("call makeOrder success");
  }

};

int main(int argc, char * argv[]) {
  if (argc != 2) {
    printf("Start test_rpc_server error, argc not 2 \n");
    printf("Start like this: \n");
    printf("./test_rpc_server ../conf/tinyrpc.xml \n");
    return 0;
  }
  tinyrpc::Config::SetGlobalConfig(argv[1]);

  tinyrpc::Logger::InitGlobalLogger();

  std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
  tinyrpc::RpcDispatcher::GetRpcDispatcher()->registerService(service);

  tinyrpc::IPNetAddr::s_ptr addr = std::make_shared<tinyrpc::IPNetAddr>("127.0.0.1", tinyrpc::Config::GetGlobalConfig()->m_port);

  tinyrpc::TcpServer tcp_server(addr);

  tcp_server.Start();

  return 0;
}
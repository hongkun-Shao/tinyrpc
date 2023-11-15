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
#include "tinyrpc/net/rpc/rpc_closure.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/net/rpc/rpc_channel.h"
#include "tinyrpc/tool/zookeeper_util.h"
#include "order.pb.h"

void test_tcp_client() {

  tinyrpc::IPNetAddr::s_ptr addr = std::make_shared<tinyrpc::IPNetAddr>("127.0.0.1", 12346);
  tinyrpc::TcpClient client(addr);
  client.Connect([addr, &client]() {
    DEBUGLOG("conenct to [%s] success", addr->ToString().c_str());
    std::shared_ptr<tinyrpc::TinyPBProtocol> message = std::make_shared<tinyrpc::TinyPBProtocol>();
    message->m_msg_id_ = "99998888";
    message->m_pb_data_ = "test pb data";

    makeOrderRequest request;
    request.set_price(100);
    request.set_goods("apple");

    if (!request.SerializeToString(&(message->m_pb_data_))) {
      ERRORLOG("serilize error");
      return;
    }

    message->m_method_name_ = "Order.makeOrder";

    client.WriteMessage(message, [request](tinyrpc::AbstractProtocol::s_ptr msg_ptr) {
      DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
    });


    client.ReadMessage("99998888", [](tinyrpc::AbstractProtocol::s_ptr msg_ptr) {
      std::shared_ptr<tinyrpc::TinyPBProtocol> message = std::dynamic_pointer_cast<tinyrpc::TinyPBProtocol>(msg_ptr);
      DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id_.c_str(), message->m_pb_data_.c_str());
      makeOrderResponse response;

      if(!response.ParseFromString(message->m_pb_data_)) {
        ERRORLOG("deserialize error");
        return;
      }
      DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
    });
  });
}

void test_rpc_channel() {
  tinyrpc::ZkClient zookeeper_client;
  zookeeper_client.Start();
  std::string service_netaddr = zookeeper_client.GetData("/Order");
  INFOLOG("Get the NetAddr where Service is located, IPV4Addr: %s", service_netaddr.c_str());
  NEWRPCCHANNEL(service_netaddr, channel);

  NEWMESSAGE(makeOrderRequest, request);
  NEWMESSAGE(makeOrderResponse, response);
  request->set_price(100);
  request->set_goods("apple");

  NEWRPCCONTROLLER(controller);
  controller->SetMsgId("99998888");
  controller->SetTimeout(10000);

  std::shared_ptr<tinyrpc::RpcClosure> closure = std::make_shared<tinyrpc::RpcClosure>([request, response, channel, controller]() mutable {
    if (controller->GetErrorCode() == 0) {
      INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
      // 执行业务逻辑
      if (response->order_id() == "xxx") {
        // xx
      }
    } else {
      ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]", 
        request->ShortDebugString().c_str(), 
        controller->GetErrorCode(), 
        controller->GetErrorInfo().c_str());
    }

    INFOLOG("now exit eventloop");
    //channel->getTcpClient()->stop();
    channel.reset();
  });

  // channel->Init(controller, request, response, closure);

  // Order_Stub stub(channel.get());

  // stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
  CALLRPRC(service_netaddr, Order_Stub, makeOrder, controller, request, response, closure); 
}

int main() {

  tinyrpc::Config::SetGlobalConfig(NULL);

  tinyrpc::Logger::InitGlobalLogger(0);

  //test_tcp_client();
  test_rpc_channel();
  INFOLOG("test_rpc_channel end");

  return 0;
}

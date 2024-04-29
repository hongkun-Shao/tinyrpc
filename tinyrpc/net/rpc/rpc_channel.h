#ifndef TINYRPC_NET_RPC_RPC_CHANNEL_H
#define TINYRPC_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>

#include <memory>

#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/timer_event.h"

namespace tinyrpc {

#define NEWMESSAGE(type, var_name) \
  std::shared_ptr<type> var_name = std::make_shared<type>();

#define NEWRPCCONTROLLER(var_name)                   \
  std::shared_ptr<tinyrpc::RpcController> var_name = \
      std::make_shared<tinyrpc::RpcController>();

#define NEWRPCCHANNEL(addr, var_name)             \
  std::shared_ptr<tinyrpc::RpcChannel> var_name = \
      std::make_shared<tinyrpc::RpcChannel>(      \
          std::make_shared<tinyrpc::IPNetAddr>(addr));

#define CALLRPRC(addr, stub_name, method_name, controller, request, response, \
                 closure)                                                     \
  {                                                                           \
    NEWRPCCHANNEL(addr, channel);                                             \
    channel->Init(controller, request, response, closure);                    \
    stub_name(channel.get())                                                  \
        .method_name(controller.get(), request.get(), response.get(),         \
                     closure.get());                                          \
  }

class RpcChannel : public google::protobuf::RpcChannel,
                   public std::enable_shared_from_this<RpcChannel> {
 public:
  typedef std::shared_ptr<RpcChannel> s_ptr;
  typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
  typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
  typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;

 public:
  RpcChannel(NetAddr::s_ptr peer_addr);
  ~RpcChannel();

  void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res,
            closure_s_ptr done);

  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* reponse,
                  google::protobuf::Closure* done);

  google::protobuf::RpcController* getController();

  google::protobuf::Message* getRequest();

  google::protobuf::Message* getResponse();

  google::protobuf::Closure* getClosure();

  TcpClient* getTcpClient();

  TimerEvent::s_ptr getTimerEvent();

 private:
  NetAddr::s_ptr m_peer_addr_{nullptr};
  NetAddr::s_ptr m_loacl_addr_{nullptr};

  controller_s_ptr m_controller_{nullptr};
  message_s_ptr m_request_{nullptr};
  message_s_ptr m_response_{nullptr};
  closure_s_ptr m_closure_{nullptr};

  bool m_is_init_{false};

  TcpClient::s_ptr m_client_{nullptr};

  TimerEvent::s_ptr m_timer_event_{nullptr};
};

}  // namespace tinyrpc

#endif
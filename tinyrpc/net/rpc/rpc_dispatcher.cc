#include "tinyrpc/net/rpc/rpc_dispatcher.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>

#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/tool/error_code.h"
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/run_time.h"
namespace tinyrpc {

static RpcDispatcher* g_rpc_dispatcher = NULL;

RpcDispatcher* RpcDispatcher::GetRpcDispatcher() {
  if (g_rpc_dispatcher != NULL) {
    return g_rpc_dispatcher;
  }
  g_rpc_dispatcher = new RpcDispatcher;
  return g_rpc_dispatcher;
}

void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request,
                             AbstractProtocol::s_ptr response,
                             TcpConnection* connection) {
  std::shared_ptr<TinyPBProtocol> req_protocol =
      std::dynamic_pointer_cast<TinyPBProtocol>(request);
  std::shared_ptr<TinyPBProtocol> rsp_protocol =
      std::dynamic_pointer_cast<TinyPBProtocol>(response);

  std::string method_full_name = req_protocol->m_method_name_;
  std::string service_name;
  std::string method_name;

  rsp_protocol->m_msg_id_ = req_protocol->m_msg_id_;
  rsp_protocol->m_method_name_ = req_protocol->m_method_name_;

  if (!parseServiceFullName(method_full_name, service_name, method_name)) {
    setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME,
                   "parse service name error");
    return;
  }

  auto it = m_service_map.find(service_name);
  if (it == m_service_map.end()) {
    ERRORLOG("%s | sericve name[%s] not found", req_protocol->m_msg_id_.c_str(),
             service_name.c_str());
    setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
    return;
  }

  service_s_ptr service = (*it).second;

  const google::protobuf::MethodDescriptor* method =
      service->GetDescriptor()->FindMethodByName(method_name);
  if (method == NULL) {
    ERRORLOG("%s | method neame[%s] not found in service[%s]",
             req_protocol->m_msg_id_.c_str(), method_name.c_str(),
             service_name.c_str());
    setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
    return;
  }

  google::protobuf::Message* req_msg =
      service->GetRequestPrototype(method).New();

  // 反序列化，将 pb_data 反序列化为 req_msg
  if (!req_msg->ParseFromString(req_protocol->m_pb_data_)) {
    ERRORLOG("%s | deserilize error", req_protocol->m_msg_id_.c_str(),
             method_name.c_str(), service_name.c_str());
    setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
    if (req_msg != NULL) {
      delete req_msg;
      req_msg = NULL;
    }
    return;
  }

  INFOLOG("%s | get rpc request[%s]", req_protocol->m_msg_id_.c_str(),
          req_msg->ShortDebugString().c_str());

  google::protobuf::Message* rsp_msg =
      service->GetResponsePrototype(method).New();

  RpcController rpcController;
  rpcController.SetLocalAddr(connection->get_local_addr());
  rpcController.SetPeerAddr(connection->get_peer_addr());
  rpcController.SetMsgId(req_protocol->m_msg_id_);

  RunTime::GetRunTime()->m_msgid = req_protocol->m_msg_id_;
  RunTime::GetRunTime()->m_method_name = method_name;

  service->CallMethod(method, &rpcController, req_msg, rsp_msg, NULL);

  if (!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data_))) {
    ERRORLOG("%s | serilize error, origin message [%s]",
             req_protocol->m_msg_id_.c_str(),
             rsp_msg->ShortDebugString().c_str());
    setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "serilize error");

    if (req_msg != NULL) {
      delete req_msg;
      req_msg = NULL;
    }
    if (rsp_msg != NULL) {
      delete rsp_msg;
      rsp_msg = NULL;
    }
    return;
  }

  rsp_protocol->m_err_code_ = 0;
  INFOLOG("%s | dispatch success, requesut[%s], response[%s]",
          req_protocol->m_msg_id_.c_str(), req_msg->ShortDebugString().c_str(),
          rsp_msg->ShortDebugString().c_str());

  delete req_msg;
  delete rsp_msg;
  req_msg = NULL;
  rsp_msg = NULL;
}

bool RpcDispatcher::parseServiceFullName(const std::string& full_name,
                                         std::string& service_name,
                                         std::string& method_name) {
  if (full_name.empty()) {
    ERRORLOG("full name empty");
    return false;
  }
  size_t i = full_name.find_first_of(".");
  if (i == full_name.npos) {
    ERRORLOG("not find . in full name [%s]", full_name.c_str());
    return false;
  }
  service_name = full_name.substr(0, i);
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);

  INFOLOG("parse sericve_name[%s] and method_name[%s] from full name [%s]",
          service_name.c_str(), method_name.c_str(), full_name.c_str());

  return true;
}

void RpcDispatcher::registerService(service_s_ptr service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_service_map[service_name] = service;
}

void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg,
                                   int32_t err_code,
                                   const std::string err_info) {
  msg->m_err_code_ = err_code;
  msg->m_err_info_ = err_info;
  msg->m_err_info_len_ = err_info.length();
}

}  // namespace tinyrpc
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "tinyrpc/net/rpc/rpc_channel.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/msg_id_util.h"
#include "tinyrpc/tool/error_code.h"
#include "tinyrpc/net/timer_event.h"
namespace tinyrpc{

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr_(peer_addr) {
    m_client_ = std::make_shared<TcpClient>(m_peer_addr_);
}

RpcChannel::~RpcChannel() {
    INFOLOG("~RpcChannel");
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method, google::protobuf::RpcController* controller, 
                            const google::protobuf::Message* request, google::protobuf::Message* response, google::protobuf::Closure* done) {

    std::shared_ptr<tinyrpc::TinyPBProtocol> req_protocol = std::make_shared<tinyrpc::TinyPBProtocol>();

    RpcController * my_controller = dynamic_cast<RpcController*>(controller);
    if(my_controller == NULL){ 
        ERRORLOG("failed callmethod, RpcController convert error");
        return;
    }

    if(my_controller->GetMsgId().empty()){
        req_protocol->m_msg_id_ = MsgIDUtil::GetMsgID();
        my_controller->SetMsgId(req_protocol->m_msg_id_);
    }else{
        req_protocol->m_msg_id_ = my_controller->GetMsgId();
    }

    req_protocol->m_method_name_ = method->full_name();
    INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id_.c_str(), req_protocol->m_method_name_.c_str());

    if(!m_is_init_){
        std::string err_info = "RpcChannel not init";
        my_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
        ERRORLOG("%s | %s, RpcChannel not init ", req_protocol->m_msg_id_.c_str(), err_info.c_str());
        return;
    }

    // requeset 的序列化
    if(!request->SerializeToString(&(req_protocol->m_pb_data_))){
        std::string err_info = "failed to serialize";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        ERRORLOG("%s | %s, origin requeset [%s] ", req_protocol->m_msg_id_.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
        return;
    }

    s_ptr channel = shared_from_this();

    m_timer_event_ = std::make_shared<TimerEvent>(my_controller->GetTimeout(), false, [my_controller, channel]() mutable {
        my_controller->StartCancel();
        my_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(my_controller->GetTimeout()));

        if (channel->getClosure()) {
            channel->getClosure()->Run();
        }
        channel.reset();
    });

    m_client_->addTimerEvent(m_timer_event_);

    m_client_->Connect([req_protocol, channel]() mutable{

        RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());

        if (channel->getTcpClient()->getConnectErrorCode() != 0) {
            my_controller->SetError(channel->getTcpClient()->getConnectErrorCode(), channel->getTcpClient()->getConnectErrorInfo());
            ERRORLOG("%s | connect error, error coode[%d], error info[%s], peer addr[%s]", 
                      req_protocol->m_msg_id_.c_str(), my_controller->GetErrorCode(), 
                      my_controller->GetErrorInfo().c_str(), channel->getTcpClient()->getPeerAddr()->ToString().c_str());
            return;
        } 
        INFOLOG("%s | connect success, peer addr[%s], local addr[%s]",
                req_protocol->m_msg_id_.c_str(), 
                channel->getTcpClient()->getPeerAddr()->ToString().c_str(), 
                channel->getTcpClient()->getLocalAddr()->ToString().c_str()); 

        channel->getTcpClient()->WriteMessage(req_protocol, [req_protocol, channel, my_controller](AbstractProtocol::s_ptr) mutable {
            INFOLOG("%s | send rpc request success. call method name[%s], peer addr[%s], local addr[%s]", 
                req_protocol->m_msg_id_.c_str(), req_protocol->m_method_name_.c_str(),
                channel->getTcpClient()->getPeerAddr()->ToString().c_str(), channel->getTcpClient()->getLocalAddr()->ToString().c_str());

            channel->getTcpClient()->ReadMessage(req_protocol->m_msg_id_, [channel, my_controller](AbstractProtocol::s_ptr msg) mutable {
                std::shared_ptr<tinyrpc::TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<tinyrpc::TinyPBProtocol>(msg);
                INFOLOG("%s | success get rpc response, call method name[%s], peer addr[%s], local addr[%s]", 
                        rsp_protocol->m_msg_id_.c_str(), rsp_protocol->m_method_name_.c_str(),
                        channel->getTcpClient()->getPeerAddr()->ToString().c_str(), channel->getTcpClient()->getLocalAddr()->ToString().c_str());
                
                  // 当成功读取到回包后， 取消定时任务
                channel->getTimerEvent()->set_cancle(true);
                if(!(channel->getResponse()->ParseFromString(rsp_protocol->m_pb_data_))){
                    ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id_.c_str());
                    my_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
                    return;
                }

                if(rsp_protocol->m_err_code_ != 0){
                    ERRORLOG("%s | call rpc methood[%s] failed, error code[%d], error info[%s]", 
                            rsp_protocol->m_msg_id_.c_str(), rsp_protocol->m_method_name_.c_str(),
                            rsp_protocol->m_err_code_, rsp_protocol->m_err_info_.c_str());

                    my_controller->SetError(rsp_protocol->m_err_code_, rsp_protocol->m_err_info_);
                    return;
                }

                INFOLOG("%s | call rpc success, call method name[%s], peer addr[%s], local addr[%s]",
                        rsp_protocol->m_msg_id_.c_str(), rsp_protocol->m_method_name_.c_str(),
                        channel->getTcpClient()->getPeerAddr()->ToString().c_str(), channel->getTcpClient()->getLocalAddr()->ToString().c_str())

                if(!my_controller->IsCanceled() && channel->getClosure()){
                    channel->getClosure()->Run();
                }

                channel.reset();
            });
        });
    });
}

void RpcChannel::Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done){
    if(m_is_init_){
        return;
    }
    m_controller_ = controller;
    m_request_ = req; 
    m_response_ = res;
    m_closure_ = done;
    m_is_init_ = true;
}

google::protobuf::RpcController* RpcChannel::getController() {
    return m_controller_.get();
}

google::protobuf::Message* RpcChannel::getRequest() {
    return m_request_.get();
}

google::protobuf::Message* RpcChannel::getResponse() {
    return m_response_.get();
}

google::protobuf::Closure* RpcChannel::getClosure() {
    return m_closure_.get();
}


TcpClient* RpcChannel::getTcpClient() {
    return m_client_.get();
}

TimerEvent::s_ptr RpcChannel::getTimerEvent() {
  return m_timer_event_;
}

}

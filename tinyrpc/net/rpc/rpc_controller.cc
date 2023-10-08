#include "tinyrpc/net/rpc/rpc_controller.h"

namespace tinyrpc{

void RpcController::Reset() {
    m_error_code_ = 0;
    m_error_info_ = "";
    m_msg_id_ = "";
    m_is_failed_ = false;
    m_is_cancled_ = false;
    m_local_addr_ = nullptr;
    m_peer_addr_ = nullptr;
    m_timeout_ = 1000;   // ms
}

bool RpcController::Failed() const {
    return m_is_failed_;
}

std::string RpcController::ErrorText() const {
    return m_error_info_;
}

void RpcController::StartCancel() {
    m_is_cancled_ = true;
}

void RpcController::SetFailed(const std::string& reason) {
    m_error_info_ = reason;
}

bool RpcController::IsCanceled() const {
    return m_is_cancled_;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {

}


void RpcController::SetError(int32_t error_code, const std::string error_info) {
    m_error_code_ = error_code;
    m_error_info_ = error_info;
    m_is_failed_ = true;
}

int32_t RpcController::GetErrorCode() {
    return m_error_code_;
}

std::string RpcController::GetErrorInfo() {
    return m_error_info_;
}

void RpcController::SetMsgId(const std::string& msg_id) {
    m_msg_id_ = msg_id;
}

std::string RpcController::GetMsgId() {
    return m_msg_id_;
}

void RpcController::SetLocalAddr(NetAddr::s_ptr addr) {
    m_local_addr_ = addr;
}

void RpcController::SetPeerAddr(NetAddr::s_ptr addr) {
    m_peer_addr_ = addr;
}

NetAddr::s_ptr RpcController::GetLocalAddr() {
    return m_local_addr_;
}

NetAddr::s_ptr RpcController::GetPeerAddr() {
    return m_peer_addr_;
}

void RpcController::SetTimeout(int timeout) {
    m_timeout_ = timeout;
}

int RpcController::GetTimeout() {
    return m_timeout_;
}

}

#ifndef TINYRPC_NET_RPC_RPC_CLOSURE_H
#define TINYRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>

#include <functional>

namespace tinyrpc {

class RpcClosure : public google::protobuf::Closure {
 public:
  RpcClosure(std::function<void()> cb) : m_cb(cb) {}

  void Run() override {
    if (m_cb != nullptr) {
      m_cb();
    }
  }

 private:
  std::function<void()> m_cb{nullptr};
};

}  // namespace tinyrpc
#endif
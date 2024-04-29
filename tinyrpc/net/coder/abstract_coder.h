#ifndef TINYRPC_NET_CODER_ABSTRACT_CODER_H
#define TINYRPC_NET_CODER_ABSTRACT_CODER_H

#include <vector>

#include "tinyrpc/net/coder/abstract_protocol.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace tinyrpc {

class AbstractCoder {
 public:
  // transfer message object to byte stream, write into buffer
  virtual void Encode(std::vector<AbstractProtocol::s_ptr>& message,
                      TcpBuffer::s_ptr out_buffer) = 0;

  // transfer the bytes stream in buffer to message onject
  virtual void Decode(std::vector<AbstractProtocol::s_ptr>& out_message,
                      TcpBuffer::s_ptr buffer) = 0;

  virtual ~AbstractCoder() {}
};

}  // namespace tinyrpc

#endif
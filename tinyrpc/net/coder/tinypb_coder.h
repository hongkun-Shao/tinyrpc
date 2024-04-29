#ifndef TINYRPC_NET_CODER_TINYPB_CODER_H
#define TINYRPC_NET_CODER_TINYPB_CODER_H

#include "tinyrpc/net/coder/abstract_coder.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"

namespace tinyrpc {

class TinyPBCoder : public AbstractCoder {
 public:
  TinyPBCoder() {}
  ~TinyPBCoder() {}

  void Encode(std::vector<AbstractProtocol::s_ptr>& messages,
              TcpBuffer::s_ptr out_buffer);

  void Decode(std::vector<AbstractProtocol::s_ptr>& out_messages,
              TcpBuffer::s_ptr buffer);

 private:
  const char* EncodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);
};

}  // namespace tinyrpc

#endif
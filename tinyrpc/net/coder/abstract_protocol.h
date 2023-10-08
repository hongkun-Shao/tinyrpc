#ifndef TINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#define TINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>

namespace tinyrpc{

class AbstractProtocol : std::enable_shared_from_this<AbstractProtocol>{
public:
  typedef std::shared_ptr<AbstractProtocol> s_ptr;

  virtual ~AbstractProtocol() {}

public:
  std::string m_msg_id_;  //req_id, unique mask

};

}


#endif
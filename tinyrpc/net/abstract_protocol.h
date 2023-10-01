#ifndef TINYRPC_NET_ABSTRACT_PROTOCOL_H
#define TINYRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace tinyrpc{

class AbstractProtocol{
 public:
  typedef std::shared_ptr<AbstractProtocol> s_ptr;
};

}


#endif
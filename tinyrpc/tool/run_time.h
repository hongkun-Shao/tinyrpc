#ifndef TINYRPC_TOOL_RUN_TIME_H
#define TINYRPC_TOOL_RUN_TIME_H


#include <string>

namespace tinyrpc {

class RunTime {
 public:

 public:
  static RunTime* GetRunTime();

 public:
  std::string m_msgid;
  std::string m_method_name;

};

}


#endif
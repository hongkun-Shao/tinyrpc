#ifndef TINYRPC_TOOL_CONFIG_H
#define TINYRPC_TOOL_CONFIG_H

#include <string>
#include <map>

namespace tinyrpc{

class Config{
public:
    Config(const char * xmlfile);
public:
    static Config * GetGlobalConfig();
    static void SetGlobalConfig(const char * xmlfile);
public:
    std::string m_log_level_;
};

}

#endif
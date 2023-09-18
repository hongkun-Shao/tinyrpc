#ifndef TINYRPC_TOOL_LOG_H
#define TINYRPC_TOOL_LOG_H

#include <string>
#include <queue>
#include <memory>

#include "tinyrpc/tool/config.h"
#include "tinyrpc/tool/mutex.h"

namespace tinyrpc{


// According to str, format args
template<typename... Args>
std::string FormatString(const char * str, Args &&... args){
    int size = snprintf(nullptr, 0, str, args...);

    std::string res;
    if(size > 0){
        res.resize(size);
        snprintf(&res[0], size + 1, str, args...);
    }

    return res;
}

#define DEBUGLOG(str, ...) \
  if (tinyrpc::Logger::GetGlobalLogger()->get_set_level() <= tinyrpc::Debug) \
  { \
    tinyrpc::Logger::GetGlobalLogger()->PushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Debug))->ToString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::FormatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->Log();                                                                                \
  } \

#define INFOLOG(str, ...) \
  if (tinyrpc::Logger::GetGlobalLogger()->get_set_level() <= tinyrpc::Info) \
  { \
    tinyrpc::Logger::GetGlobalLogger()->PushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Info))->ToString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::FormatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->Log();                                                                                \
  } \


#define ERRORLOG(str, ...) \
  if (tinyrpc::Logger::GetGlobalLogger()->get_set_level() <= tinyrpc::Error) \
  { \
    tinyrpc::Logger::GetGlobalLogger()->PushLog((new tinyrpc::LogEvent(tinyrpc::LogLevel::Error))->ToString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyrpc::FormatString(str, ##__VA_ARGS__) + "\n");\
    tinyrpc::Logger::GetGlobalLogger()->Log();                                                                                \
  } \



enum LogLevel{
    Unkown = 0,
    Debug,
    Info,
    Error
};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string& log_level);

class LogEvent{
public:
    LogEvent(LogLevel level) : m_log_level_(level){}
    std::string get_file_name() const {
        return m_file_name_;
    }
    
    LogLevel get_log_level() const {
        return m_log_level_;
    }

    std::string ToString();
private:
    std::string m_file_name_;
    int32_t m_file_line_;
    int32_t m_pid_;
    int32_t m_thread_id_;
    LogLevel m_log_level_;
};

class Logger{
public:
    //typedef std::shared_ptr<Logger> s_ptr;

    Logger(LogLevel level) : m_set_level_(level){}

    void PushLog(const std::string & msg);

    void Log();

    LogLevel get_set_level() const {
        return m_set_level_;
    }
public:
    // global singleton pattern
    static Logger * GetGlobalLogger();

    static void InitGlobalLogger();

private:
    LogLevel m_set_level_;                  // limit the log whose level < set_level
    std::queue<std::string> m_log_buffer_;  // log queue

    //Mutex
};




}

#endif
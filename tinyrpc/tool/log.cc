#include <sstream>
#include <sys/time.h>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/util.h"
namespace tinyrpc{

//------------------------------------------------------------------
//enum  LogLevel                                                   +
//------------------------------------------------------------------

std::string LogLevelToString(LogLevel level){
    switch(level){
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Error:
        return "ERROR";
    default:
        return "UNKOWN";
    }
}


LogLevel StringToLogLevel(const std::string& log_level) {
    if(log_level == "DEBUG"){
        return Debug;
    }else if(log_level == "INFO"){
        return Info;
    }else if(log_level == "ERROR"){
        return Error;
    }else{
        return Unkown;
    }
}

//------------------------------------------------------------------
//class  LogEvent                                                   +
//------------------------------------------------------------------
std::string LogEvent::ToString(){
    struct timeval now_time_t;

    gettimeofday(&now_time_t, nullptr);

    struct tm now_time;
    localtime_r(&(now_time_t.tv_sec), &now_time);

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time);
    std::string time_str(buf);
    int ms = now_time_t.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);


    m_pid_ = GetPid();
    m_thread_id_ = GetThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_log_level_) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid_ << ":" << m_thread_id_ << "]\t";
    
    return ss.str();
}


//------------------------------------------------------------------
//class  Logger                                                  +
//------------------------------------------------------------------

static Logger * g_logger = nullptr;

Logger * Logger::GetGlobalLogger(){
    if(g_logger){
        return g_logger;
    }
    InitGlobalLogger();
    return g_logger;
}

void Logger::InitGlobalLogger(){
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level_);
    printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
    g_logger = new Logger(global_log_level);
}

void Logger::PushLog(const std::string & msg){
    Locker<Mutex> lock(m_mutex_);
    m_log_buffer_.push(msg);
    lock.unlock();
}

void Logger::Log(){
    Locker<Mutex> lock(m_mutex_);
    std::queue<std::string> temp;
    m_log_buffer_.swap(temp);
    lock.unlock();

    while(!temp.empty()){
        std::string msg = temp.front();
        temp.pop();
        printf(msg.c_str());
    }
}



}




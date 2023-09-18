#include <sys/syscall.h>
#include "tinyrpc/tool/util.h"

namespace tinyrpc{

static int g_pid = 0;

static thread_local int g_thread_id = 0;

pid_t GetPid(){
    if(g_pid){
        return g_pid;
    }
    return getpid();
}

pid_t GetThreadId(){
    if(g_thread_id){
        return g_thread_id;
    }
    return syscall(SYS_gettid);
}
} // namespace tinyrpc

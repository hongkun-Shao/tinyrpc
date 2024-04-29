#include <sys/types.h>
#include <unistd.h>

namespace tinyrpc {

pid_t GetPid();

pid_t GetThreadId();

int64_t GetNowMs();

int32_t GetInt32FromNetByte(const char* buf);

}  // namespace tinyrpc
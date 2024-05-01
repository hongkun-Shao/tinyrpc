#include "tinyrpc/tool/util.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace tinyrpc {

static int g_pid = 0;

static thread_local int g_thread_id = 0;

pid_t GetPid() {
  if (g_pid) {
    return g_pid;
  }
  return getpid();
}

pid_t GetThreadId() {
  if (g_thread_id) {
    return g_thread_id;
  }
  return syscall(SYS_gettid);
}

int64_t GetNowMs() {
  timeval ts;
  gettimeofday(&ts, NULL);

  return ts.tv_sec * 1000 + ts.tv_usec / 1000;
}

int32_t GetInt32FromNetByte(const char* buf) {
  int32_t res;
  memcpy(&res, buf, sizeof(res));
  return ntohl(res);
}

}  // namespace tinyrpc

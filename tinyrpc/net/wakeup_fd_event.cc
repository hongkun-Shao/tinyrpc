#include "tinyrpc/net/wakeup_fd_event.h"

#include <unistd.h>

#include "tinyrpc/tool/log.h"

namespace tinyrpc {

WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd) {}

WakeUpFdEvent::~WakeUpFdEvent() {}

void WakeUpFdEvent::wakeup() {
  char buf[8] = {"wakeup"};
  int res = write(m_fd_, buf, 8);
  if (res != 8) {
    ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd_);
  }
  DEBUGLOG("success read 8 bytes");
}

}  // namespace tinyrpc
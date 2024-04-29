#ifndef TINYRPC_COROUTINE_MEMORY_H
#define TINYRPC_COROUTINE_MEMORY_H

#include <atomic>
#include <memory>
#include <vector>

#include "tinyrpc/tool/mutex.h"

namespace tinyrpc {

class Memory {
 public:
  typedef std::shared_ptr<Memory> ptr;

  Memory(int block_size, int block_count);

  // void free();

  ~Memory();

  const int getRefCount();

  char* getStart();

  char* getEnd();

  char* getBlock();

  void backBlock(char* s);

  void hasBlock(char* s);

 private:
  int block_size_{0};
  int block_count_{0};

  // total_size_ = block_size * block_count
  int total_size_{0};

  char* m_start{nullptr};
  char* m_end{nullptr};

  std::atomic<int> ref_counts_{0};
  std::vector<bool> blocks_;
  Mutex mutex_;
};

}  // namespace tinyrpc

#endif
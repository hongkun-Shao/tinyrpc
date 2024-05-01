#include "tinyrpc/coroutine/memory.h"

#include <assert.h>
#include <error.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <cstring>
#include <memory>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/mutex.h"

namespace tinyrpc {

Memory::Memory(int block_size, int block_count)
    : block_size_(block_size), block_count_(block_count) {
  // memory init
  total_size_ = block_count_ * block_size_;
  start_ptr_ = (char*)malloc(total_size_);
  assert(start_ptr_ != nullptr);
  INFOLOG << "Success Malloc " << total_size_ << " Bytes memory";
  end_ptr_ = start_ptr_ + total_size_ - 1;

  // blocks init
  blocks_.resize(block_count_);
  for (size_t i = 0; i < blocks_.size(); ++i) {
    blocks_[i] = false;
  }
  ref_counts_ = 0;
}

Memory::~Memory() {
  if (!start_ptr_) {
    return;
  }
  free(start_ptr_);
  INFOLOG << "Success Free " << total_size_ << "Bytes memory";
  start_ptr_ = nullptr;
  end_ptr_ = nullptr;

  ref_counts_ = 0;
}

char* Memory::getStart() { return start_ptr_; }

char* Memory::getEnd() { return end_ptr_; }

const int Memory::getRefCount() { return ref_counts_; }

char* Memory::getBlock() {
  int t = -1;
  Locker<Mutex> lock(mutex_);
  for (size_t i = 0; i < blocks_.size(); ++i) {
    if (!blocks_[i]) {
      blocks_[i] = true;
      t = i;
      break;
    }
  }
  lock.unlock();
  if (t == -1) {
    return nullptr;
  }

  ref_counts_++;
  return start_ptr_ + (t * block_size_);
}

void Memory::backBlock(char* s) {
  if (s > end_ptr_ || s < start_ptr_) {
    ERRORLOG << "back block error, this block is not belong to this Memory";
    return;
  }
  int index = (s - start_ptr_) / block_size_;
  Locker<Mutex> lock(mutex_);
  blocks_[index] = false;
  lock.unlock();
  ref_counts_--;
}

bool Memory::hasBlock(char* s) {
  return ((s >= start_ptr_) && (s <= end_ptr_));
}

}  // namespace tinyrpc
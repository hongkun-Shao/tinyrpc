#include <memory>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include "tinyrpc/tool/log.h"
#include "tinyrpc/coroutine/memory.h"

namespace tinyrpc {

Memory::Memory(int block_size, int block_count) : 
              block_size_(block_size), block_count_(block_count) {
                
}

}
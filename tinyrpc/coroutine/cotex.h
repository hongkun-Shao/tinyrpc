#ifndef TINYRPC_COROUTINE_COCTX_H
#define TINYRPC_COROUTINE_COCTX_H

namespace tinyrpc {
enum {
  kBRP = 3,      // rbp, the bottom of stack
  KRDI = 4,      // rdi, the first para of the call function
  KRSI = 5,      // rsi, the second para of thr call function
  KRETADDR = 9,  // the next excute cmd address, it will be assigned to rip
  KRSP = 13,     // rsp, top of stack
};

struct coctx {
  void* regs[14];  // 14 registers
};

extern "C" {
// 1. save the current registers to the first coctx
// 2. assign the second coctx'register to the current registers
extern void coctx_swap(coctx*, coctx*) asm("coctx_swap");
};

}  // namespace tinyrpc

#endif
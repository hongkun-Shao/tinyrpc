#include "tinyrpc/coroutine/coroutine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

#include "tinyrpc/tool/log.h"
#include "tinyrpc/tool/run_time.h"

namespace tinyrpc {

// main Coroutine, Every IO thread has a main Coroutine
static thread_local Coroutine* t_main_coroutine = nullptr;

// the current running Coroutine in the current thread
static thread_local Coroutine* t_current_coroutine = nullptr;

static thread_local RunTime* t_current_run_time = nullptr;

static std::atomic<int> t_coroutine_count{0};

static std::atomic<int> t_current_coroutine_id{1};

int getCoroutineIndex() { return t_current_coroutine_id; }

RunTime* getCurrentRunTime() { return t_current_run_time; }

void setCurrentRunTime(RunTime* val) { t_current_run_time = val; }

void CoFunction(Coroutine* cor) {
  if (cor != nullptr) {
    cor->setIsInCoFunc(true);

    // excute call_back of coroutine
    cor->call_back_();

    cor->setIsInCoFunc(false);
  }

  // call_back finished, coroutine life is over
  Coroutine::Yield();
}

Coroutine::Coroutine() {
  // main coroutine's id is 0
  id_ = 0;
  t_coroutine_count++;
  memset(&coctx_, 0, sizeof(coctx_));
  t_current_coroutine = this;
}

void Coroutine::Init() {
  assert(stack_sp_);
  if (!t_main_coroutine) {
    t_main_coroutine = new Coroutine();
  }

  id_ = t_current_coroutine_id++;
  t_coroutine_count++;
}

Coroutine::Coroutine(int size, char* stack_ptr)
    : stack_size_(size), stack_sp_(stack_ptr) {
  Init();
  INFOLOG << "coroutine [" << id_ << "] success create";
}

Coroutine::Coroutine(int size, char* stack_ptr, std::function<void()> cb)
    : stack_size_(size), stack_sp_(stack_ptr) {
  Init();
  INFOLOG << "coroutine [" << id_ << "] success create";
  setCallBack(cb);
}

bool Coroutine::setCallBack(std::function<void()> cb) {
  if (this == t_main_coroutine) {
    ERRORLOG << "Main coroutine can't set callback";
    return false;
  }
  if (is_in_cofunc_) {
    ERRORLOG << "this coroutine is in CoFunction";
    return false;
  }
  call_back_ = cb;

  char* top = stack_sp_ + stack_size_;

  top = reinterpret_cast<char*>(reinterpret_cast<unsigned long>(top) & -16LL);

  memset(&coctx_, 0, sizeof(coctx_));

  coctx_.regs[KRSP] = top;
  coctx_.regs[kBRP] = top;
  coctx_.regs[KRETADDR] = reinterpret_cast<char*>(CoFunction);
  ;
  coctx_.regs[KRDI] = reinterpret_cast<char*>(this);

  can_resume_ = true;
  return true;
}

Coroutine::~Coroutine() {
  t_coroutine_count--;
  INFOLOG << "coroutine [" << id_ << "] die";
  // free(stack_sp_);
}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_current_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
    t_current_coroutine = t_main_coroutine;
  }
  return t_current_coroutine;
}

Coroutine* Coroutine::GetMainCoroutine() {
  if (t_main_coroutine) {
    return t_main_coroutine;
  }
  t_main_coroutine = new Coroutine();
  return t_main_coroutine;
}

bool Coroutine::IsMainCoroutine() {
  if (t_main_coroutine == nullptr || t_current_coroutine == t_main_coroutine) {
    return true;
  }
  return false;
}

// from target Coroutine back to Main Coroutine
void Coroutine::Yield() {
  if(t_main_coroutine == nullptr) {
    ERRORLOG << "Main Coroutine is nullptr";
    return;
  }

  if(t_current_coroutine == t_main_coroutine) {
    ERRORLOG << "Current Coroutine is already Main Coroutine";
    return;
  }

  Coroutine* cor = t_current_coroutine;
  t_current_coroutine = t_main_coroutine;
  t_current_coroutine = nullptr;
  coctx_swap(&(cor->coctx_), &(t_main_coroutine->coctx_));
}

// from Main Coroutine switch to target coroutine
void Coroutine::Resume(Coroutine* cor) {
  if(t_current_coroutine != t_main_coroutine) {
    ERRORLOG << "Resume error, current coroutine must is main coroutine";
    return;
  }
  if(!t_main_coroutine) {
    ERRORLOG << "Main coroutine is nullptr";
    return;
  }
  if(!cor || !cor->can_resume_) {
    ERRORLOG << "Pending coroutine is nullptr or can_resume is false";
    return
  }
  if(t_current_coroutine == cor) {
    INFOLOG << "Current coroutine is pending coroutine, need't swap"
    return;
  }
  t_current_coroutine = cor;
  t_current_run_time = cor->getRunTime();
  coctx_swap(&(t_main_coroutine->coctx_), &(cor->coctx_));
}

}  // namespace tinyrpc
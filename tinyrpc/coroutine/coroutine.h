#ifndef TINYRPC_COROUTINE_COROUTINE_H
#define TINYRPC_COROUTINE_COROUTINE_H

#include <functional>
#include <memory>

#include "tinyrpc/coroutine/cotex.h"
#include "tinyrpc/tool/run_time.h"

namespace tinyrpc {

int getCoroutineIndex();

RunTime* getCurrentRunTime();

void setCurrentRunTime();

class Coroutine {
 public:
  typedef std::shared_ptr<Coroutine> s_ptr;

 private:
  // to Construct Main Coroutine
  Coroutine();

 public:
  Coroutine(int size, char* stack_ptr);

  Coroutine(int size, char* stack_ptr, std::function<void()> cb);

  void Init();

  ~Coroutine();

  bool setCallBack(std::function<void()> cb);

  const int getId() const { return id_; }

  void setIsInCoFunc(const bool val) { is_in_cofunc_ = val; }

  const bool getIsInCoFunc() const { return is_in_cofunc_; }

  const std::string getMsgNo() const { return msg_no_; }

  RunTime* getRunTime() { return &run_time_; }

  void setMsgNo(const std::string& msg_no) { msg_no_ = msg_no; }

  void setIndex(const int& index) { index_ = index; }

  const int getStackSize() const { return stack_size_; }

  void setCanResum(bool val) { can_resume_ = val; }

 public:
  static void Yield();

  static void Resume(Coroutine* cor);

  static Coroutine* GetCurrentCoroutine();

  static Coroutine* GetMainCoroutine();

  static bool IsMainCoroutine();

 private:
  int id_;  // the coroutine's id
  coctx coctx_;
  int stack_size_;            // size of stack memory space
  char* stack_sp_{nullptr};   // coroutine's stack memory space
  bool is_in_cofunc_{false};  // check if in CoFuntion

  std::string msg_no_;
  RunTime run_time_;

  bool can_resume_{false};
  int index_{-1};  // index in coroutine pool

 public:
  std::function<void()> call_back_;
};

}  // namespace tinyrpc
#endif
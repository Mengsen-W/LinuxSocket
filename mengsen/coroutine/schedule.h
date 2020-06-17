/*
 * @Description: schedule
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 21:38:58
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:58:02
 */

#ifndef __MENGSEN_SCHEDULE_H__
#define __MENGSEN_SCHEDULE_H__

#define DEFAULT_COROUTINE 16

#include <ucontext.h>

#include "types.h"

namespace mengsen_co {
/**
 * 计划设计一个协程控制器
 * 控制器提供控制协程切换和管理协程的功能
 */
class schedule : public std::enable_shared_from_this<schedule> {
 public:
  schedule();
  ~schedule();
  schedule(const schedule&);
  schedule(schedule&&);
  schedule& operator=(const schedule&);
  schedule& operator=(schedule&&);

  void coroutine_close();
  size_t coroutine_create(coroutine_func, void*);
  void coroutine_resume(int);
  co_status coroutine_status(int);
  inline int coroutine_running() const { return _running; }
  void set_running(int i) { _running = i; }
  void coroutine_yield();
  coroutine_vec_ptr get_vec_ptr() const { return _co_ptr; }

 private:
  char _stack[STACK_SIZE];    // tunning time stack
  ucontext_t _main;           // main coroutine context
  coroutine_vec_ptr _co_ptr;  // array for saved coroutine
  int _running;               // running coroutine id
};

}  // namespace mengsen_co

#endif  //__MENGSEN_SCHEDULE_H__
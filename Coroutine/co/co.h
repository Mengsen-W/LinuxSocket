/*
 * @Description: encapsulation ucontext
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 21:38:58
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-30 22:09:49
 */

#ifndef __MENGSEN_CO_H__
#define __MENGSEN_CO_H__

#define STACK_SIZE (1024 * 1024)

#include <ucontext.h>

#include <functional>
#include <memory>

class co_coroutine;

namespace mengsen_co {

class co_schedule {
  typedef std::function<void(struct schedule*, void*)> coroutine_func;

  co_schedule();
  ~co_schedule();
  co_schedule(const co_schedule&);
  co_schedule(co_schedule&&);
  co_schedule& operator=(const co_schedule&);
  co_schedule& operator=(co_schedule&&);

  void coroutine_close(co_schedule&);
  int coroutine_create(co_schedule&, coroutine_func, void*);
  void coroutine_resume(co_schedule&, int);
  int coroutine_status(co_schedule&, int);
  int coroutine_running(co_schedule&);
  void coroutine_yield(co_schedule&);

 private:
  co_schedule coroutine_open(void);

  char stack[STACK_SIZE];  // tunning time stack
  ucontext_t main;         // main coroutine context
  int nco;                 // alive coroutine number
  int cap;                 // schedule capacity
  int running;             // running coroutine id
  class coroutine** co;    // array for saved coroutine
};

}  // namespace mengsen_co

#endif  //__MENGSEN_CO_H__
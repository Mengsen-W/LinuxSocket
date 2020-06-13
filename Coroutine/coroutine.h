/*
 * @Description: coroutine
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 10:35:32
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 09:36:14
 */

#ifndef __MENGSEN_COROUTINE_H__
#define __MENGSEN_COROUTINE_H__

#include <ucontext.h>

#include "types.h"

namespace mengsen_co {

class coroutine {
 public:
  coroutine();
  coroutine(schedule_ptr, coroutine_func, void*);
  ~coroutine();
  coroutine(const coroutine&);
  coroutine(coroutine&&);
  coroutine& operator=(const coroutine&);
  coroutine& operator=(coroutine&&);

  co_status get_status() const { return _status; };
  void set_status(co_status status) { _status = status; }
  ucontext_t& get_context() { return _ctx; }
  void save_stack(char*);
  static void mainfunc(schedule_ptr*);
  // TODO non-implements
  void set_func(schedule_ptr, void*);
  void* get_ud() const { return _ud; }
  ptrdiff_t get_size() const { return _size; }
  char* get_stack() const { return _stack; }
  ptrdiff_t get_cap() const { return _cap; }
  void set_cap(ptrdiff_t cap) { _cap = cap; }

 private:
  coroutine_func _func;
  void* _ud;
  ucontext_t _ctx;
  schedule_ptr _sch;
  ptrdiff_t _cap;
  ptrdiff_t _size;
  co_status _status;
  char* _stack;
};

}  // namespace mengsen_co

#endif  // __MENGSEN_COROUTINE_H__
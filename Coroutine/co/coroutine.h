/*
 * @Description: coroutine
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 10:35:32
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-31 12:56:16
 */

#ifndef __MENGSEN_COROUTINE_H__
#define __MENGSEN_COROUTINE_H__

#include <ucontext.h>

#include "types.h"

namespace mengsen_co {

class coroutine {
 public:
  explicit coroutine(schedule_ptr, coroutine_func, void*);
  ~coroutine();
  coroutine(const coroutine&);
  coroutine(coroutine&&);
  coroutine& operator=(const coroutine&);
  coroutine& operator=(coroutine&&);

  co_status status() const { return _status; };
  void save_stack(char*);
  void mainfunc(uint32_t, uint32_t);

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
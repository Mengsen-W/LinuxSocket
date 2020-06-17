/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 11:26:39
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:48:12
 */

#include "./coroutine.h"

#include <cassert>
#include <cstring>

#include "schedule.h"

namespace mengsen_co {

coroutine::coroutine()
    : _func(nullptr),
      _ud(nullptr),
      _sch(nullptr),
      _cap(0),
      _size(0),
      _status(co_status::READY),
      _stack(nullptr) {}

coroutine::coroutine(schedule_ptr s, coroutine_func func, void *ud)
    : _func(func),
      _ud(ud),
      _sch(s),
      _cap(0),
      _size(0),
      _status(co_status::READY),
      _stack(nullptr) {}

coroutine::~coroutine() {
  // TODO 更改为智能指针
  _sch.reset();
  free(_stack);
}

void coroutine::mainfunc(schedule_ptr *ptr) {
  schedule_ptr s_ptr = *ptr;

  int id = s_ptr->coroutine_running();
  coroutine_vec_ptr s_v_ptr = s_ptr->get_vec_ptr();
  coroutine_ptr c_ptr = s_v_ptr->at(id);
  c_ptr->set_func(s_ptr, c_ptr->get_ud());
  s_v_ptr->erase(s_v_ptr->begin() + id);
  s_ptr->set_running(-1);
}

void coroutine::save_stack(char *top) {
  char dummy = 0;

  assert(top - &dummy <= STACK_SIZE);
  if (_cap < top - &dummy) {
    free(_stack);
    _cap = top - &dummy;
    _stack = static_cast<char *>(malloc(_cap));
  }
  _size = top - &dummy;
  memcpy(_stack, &dummy, _size);
}

void coroutine::set_func(schedule_ptr ptr, void *ud) { _func(ptr, ud); }

}  // namespace mengsen_co
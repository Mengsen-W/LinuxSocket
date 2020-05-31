/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 11:26:39
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-31 12:09:03
 */

#include "coroutine.h"

namespace mengsen_co {

coroutine::coroutine(schedule_ptr s, coroutine_func func, void *ud)
    : _func(func),
      _ud(ud),
      _sch(s),
      _cap(0),
      _size(0),
      _status(co_status::READY),
      _stack(nullptr) {}

coroutine::~coroutine() {}

}  // namespace mengsen_co
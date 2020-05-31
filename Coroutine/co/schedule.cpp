/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 21:41:30
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-31 12:54:39
 */

#include <coroutine.h>
#include <schedule.h>

#include <cassert>

namespace mengsen_co {
schedule::schedule()
    : _co_ptr(std::make_shared<coroutine_vec>(nullptr)), _running(-1) {}

schedule::~schedule() { _co_ptr->clear(); }

int schedule::coroutine_create(coroutine_func func, void* ud) {
  coroutine_ptr co = std::make_shared<coroutine>(shared_from_this(), func, ud);
  _co_ptr->push_back(co);
  return _co_ptr->size();
}

void schedule::coroutine_resume(int id) {}

void schedule::coroutine_yield() {}

/**
 * get coroutine in schedule
 * @param: int id [coroutine id]
 * @return co_status
 */
co_status schedule::coroutine_status(int id) {
  assert(id >= 0 && id < _co_ptr->capacity());
  if (id > _co_ptr->size()) return co_status::DEAD;
  return _co_ptr->at(id)->status();
}

/**
 * get id with running coroutine
 * @param: void
 * @return: int id [coroutine id]
 */
inline int schedule::coroutine_running() const { return _running; }

}  // namespace mengsen_co
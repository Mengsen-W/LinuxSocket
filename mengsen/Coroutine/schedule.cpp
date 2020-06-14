/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 21:41:30
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:55:57
 */

#include <coroutine.h>
#include <schedule.h>

#include <cassert>
#include <cstring>  // for memcpy

namespace mengsen_co {
schedule::schedule()
    : _stack(), _co_ptr(std::make_shared<coroutine_vec>()), _running(-1) {
  memset(_stack, 0, STACK_SIZE);
}

schedule::~schedule() {
  std::vector<coroutine_ptr>::const_iterator iter;
  _co_ptr->clear();
}

/**
 * create coroutine in schedule
 * @param: coroutine_func func, void *ud [ argv[] ]
 * @return: size_t
 */
size_t schedule::coroutine_create(coroutine_func func, void* ud) {
  coroutine_ptr co = std::make_shared<coroutine>(shared_from_this(), func, ud);
  //_co_ptr->emplace_back(
  //     std::make_shared<coroutine>(shared_from_this(), func, ud));
  // return _co_ptr->size();
  return 0;
}

/**
 * switch to the corresponding coroutine
 * @param: int id [coroutine id]
 * @return: void
 */
void schedule::coroutine_resume(int id) {
  assert(_running == -1);
  assert(id >= 0 && id <= static_cast<int>(_co_ptr->size()));
  coroutine_ptr c = _co_ptr->at(id);
  if (c == nullptr) return;

  co_status status = c->get_status();
  ucontext_t uc = c->get_context();

  switch (status) {
    case co_status::READY: {
      getcontext(&uc);
      uc.uc_stack.ss_sp = _stack;
      uc.uc_stack.ss_size = STACK_SIZE;
      uc.uc_link = &_main;
      _running = id;
      c->set_status(co_status::RUNNING);

      schedule_ptr s_ptr = shared_from_this();

      makecontext(&uc, reinterpret_cast<void (*)(void)>(coroutine::mainfunc), 1,
                  &s_ptr);

      swapcontext(&_main, &uc);
      break;
    }
    case co_status::SUSPEND: {
      memcpy(_stack + STACK_SIZE - c->get_size(), c->get_stack(),
             c->get_size());
      _running = id;
      c->set_status(co_status::RUNNING);
      swapcontext(&_main, &uc);
      break;
    }
    default:
      assert(0);
  }

  return;
}

/**
 * yield running coroutine and switch to main coroutine
 * @param: void
 * @return: void
 */
void schedule::coroutine_yield() {
  int id = _running;
  assert(id >= 0);

  coroutine_ptr c = _co_ptr->at(id);
  assert(reinterpret_cast<char*>(c.get()) > _stack);

  c->save_stack(_stack + STACK_SIZE);
  c->set_status(co_status::SUSPEND);
  _running = -1;

  swapcontext(&(c->get_context()), &_main);
}

/**
 * get coroutine in schedule
 * @param: int id [coroutine id]
 * @return co_status
 */
co_status schedule::coroutine_status(int id) {
  assert(id >= 0 && id < static_cast<int>(_co_ptr->capacity()));
  if (id > static_cast<int>(_co_ptr->size())) return co_status::DEAD;
  return _co_ptr->at(id)->get_status();
}

}  // namespace mengsen_co
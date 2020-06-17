/*
 * @Descriptin: types
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 10:48:26
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:54:47
 */

#ifndef __MENGSEN_TYPES_H__
#define __MENGSEN_TYPES_H__

#include <functional>
#include <memory>
#include <vector>

namespace mengsen_co {

#define STACK_SIZE (1024 * 1024)

class schedule;
class coroutine;

enum class co_status : unsigned short { DEAD, READY, RUNNING, SUSPEND };

typedef std::shared_ptr<schedule> schedule_ptr;
typedef std::shared_ptr<coroutine> coroutine_ptr;
typedef std::function<void(schedule_ptr, void*)> coroutine_func;
typedef std::vector<coroutine_ptr> coroutine_vec;
typedef std::shared_ptr<coroutine_vec> coroutine_vec_ptr;

}  // namespace mengsen_co

#endif  // __MENGSEN_TYPES_H__
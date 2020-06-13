/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-01 09:20:08
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:36:31
 */

// for testing

#include <iostream>

#include "schedule.h"
#include "types.h"

using namespace mengsen_co;

struct args {
  int n;
};

static void foo(schedule_ptr S, void *ud) {}

static void test(schedule_ptr c) {
  args arg1 = {1};
  // args arg2 = {1000};
  std::cout << c->coroutine_running() << std::endl;

  size_t co1 = c->coroutine_create(foo, &arg1);
  std::cout << c->coroutine_running() << co1 << std::endl;
  return;
}

int main() {
  schedule_ptr c = std::make_shared<schedule>();
  test(c);
  // 调用智能指针的析构函数
  return 0;
}
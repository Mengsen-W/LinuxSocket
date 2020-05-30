/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 15:07:48
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-30 16:28:20
 * @Description: for test ucontext
 */

#include <ucontext.h>

#include <chrono>
#include <iostream>
#include <thread>

void test() {
  ucontext_t context;

  // get current thread context
  getcontext(&context);
  std::cout << "Hello context" << std::endl;
  std::cout << context.uc_stack.ss_sp << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // set context with arg1
  setcontext(&context);
  return;
}

int main() {
  test();
  return 0;
}

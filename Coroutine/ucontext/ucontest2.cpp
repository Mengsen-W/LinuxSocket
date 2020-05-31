/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-30 15:07:46
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-31 17:58:09
 * @Description: for test ucontext
 */

#include <signal.h>
#include <ucontext.h>  //include stack_t to see more information as signal.h

#include <chrono>
#include <iostream>
#include <thread>

// typedef struct ucontext_t {
//   //* some like a flags
//   unsigned long int __ctx(uc_flags);
//   //* pointer to the context that is resumed when this context is returned
//   struct ucontext_t* uc_link;
//   //* the stack used by this context
//   stack_t uc_stack;
//   //* a mechine-specific representation of the saved context
//   mcontext_t uc_mcontext;
//   //* set of signals that are blocked when this context is active
//   sigset_t uc_sigmask;
//   struct _libc_fpstate __fpregs_mem;
// } ucontext_t;

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

void fun(int i, const char* j, const char* k) {
  std::cout << "call from context to function = " << i << ' ' << j << ' ' << k
            << ' ' << std::endl;
  return;
}

void context_test() {
  char stack_stk[1024 * 128] = {0};

  // current context
  ucontext_t child;

  // previous context
  ucontext_t main;

  getcontext(&child);
  std::cout << child.uc_stack.ss_sp << std::endl;
  // set stack
  // fun is not stack address when swap to child context
  // because that is a not used function
  // so there is no stack frame
  // address crossing the line
  child.uc_stack.ss_sp = stack_stk;
  child.uc_stack.ss_size = sizeof(stack_stk);
  child.uc_stack.ss_flags = SS_ONSTACK;

  // resume when child's function returns
  // if set nullptr child's function don't save previous context
  // it just return and shutdown all of process/thread
  child.uc_link = &main;

  std::cout << child.uc_stack.ss_sp << std::endl;
  // __argc in number of function parameter
  // in this case put on three parameter, respectively 1, "hello", "world"
  // argv just pass pointer's type uint32_t
  // but in 64 bit os pointer is uint32_t
  makecontext(&child, (void (*)())fun, 3, 1, "hello", "world");

  // set current context to main and swap to child context
  swapcontext(&main, &child);

  std::cout << "back to main context" << std::endl;
  return;
}

int main() {
  context_test();
  std::cout << "finish context test" << std::endl;
  return 0;
}

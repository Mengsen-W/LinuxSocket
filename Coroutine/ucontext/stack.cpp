/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-31 15:40:00
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-31 16:17:17
 */

#include <cstdio>
#include <iostream>

void test(char* stack) {
  char dummy1 = 0;
  printf("second = %p\n", &dummy1);
  printf("first = %p\n", stack);
}

int main() {
  char stack[1024] = {};
  printf("first = %p\n", stack);
  printf("last element = %p\n", &stack[1024]);
  test(stack);
  printf("%p\n", &stack[0]);
  printf("%p\n", &stack[1]);
  printf("%p\n", &stack[2]);
  printf("%p\n", &stack[3]);
  // std::cout << (stack + 1024) - &dummy << std::endl;
  return 0;
}
/**
first = 0x7fff40952320
last element = 0x7fff40952720
second = 0x7fff40952290
first = 0x7fff40952320
0x7fff40952320
0x7fff40952321
0x7fff40952322
0x7fff40952323
*/

/**
 * 在Linux栈中 栈空间地址由高向低增长
 * 后声明的变量地址最低
 * 对于数组来说 从低地址向高地址增长
 * 也就是由栈顶向栈底增长
 */
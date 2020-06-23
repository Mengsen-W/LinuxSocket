/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-23 17:13:31
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-23 17:27:09
 */

#include <cstdio>

#include "signal.h"
#include "unistd.h"

void handler(int s) {
  printf("SIGHUP\n");
  return;
}

int main() {
  signal(SIGHUP, handler);
  // int a = 0;
  // while 是符合汇编逻辑的
  // 如果在判断那么下一条是跳转这个时候跳转一定会跳转到loop
  // 如果在跳转那么下一条是判断loop
  while (true)
    ;
  printf("sig\n");

  return 0;
}
// while 不是原子操作
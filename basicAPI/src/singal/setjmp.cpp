/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-23 18:10:40
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-23 19:09:02
 */

#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#include <cstdio>

sigjmp_buf buf;

void handler(int dummy) { siglongjmp(buf, 1); }

int main() {
  if (!sigsetjmp(buf, 1)) {
    // 这里屏蔽字在整个栈空间是唯一的
    // 如果是setjmp只是单纯的恢复信息
    // sigsetjmp唯独不恢复信号部分
    signal(SIGINT, handler);
    sleep(10);
    printf("starting\n");
  } else {
    printf("restarting\n");
  }

  while (true) {
    sleep(1);
    printf("processing...\n");
  }
  return 0;
}

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-26 11:34:42
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-26 11:49:57
 * @Description: 多线程读写文件观察seek是否改变
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

int main() {
  int fd;
  char c;

  fd = open("foobar.txt", O_RDONLY, 0);

  if (fork() == 0) {
    //! 若先关了再打开则子进程拥有自己的打开文件表项
    close(fd);
    fd = open("foobar.txt", O_RDONLY, 0);
    //! 否则子进程和父进程共享一个打开文件表项
    lseek(fd, 2, 0);
    read(fd, &c, 1);
    printf("c = %c\n", c);
    sleep(2);
    read(fd, &c, 1);
    printf("c = %c\n", c);
    return 0;
  }

  sleep(1);
  lseek(fd, 0, 0);
  wait(NULL);

  // printf("c = %c\n", c);
  close(fd);
  return 0;
}

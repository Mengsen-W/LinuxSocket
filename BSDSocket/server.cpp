/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-17 21:21:34
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-17 21:57:06
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv) {
  char hello[] = "hello world";
  // 创建 Socket协议对象
  struct sockaddr_in sa;
  // 创建 socket 对象 监听套接字
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD) {
    // 创建失败
    perror("cannot creat socket");
    exit(EXIT_FAILURE);
  }

  // 初始化协议
  memset(&sa, 0, sizeof(sa));
  // 定义协议
  sa.sin_family = AF_INET;
  sa.sin_port = htons(22222);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  // 绑定socket
  if (-1 == bind(SocketFD, (struct sockaddr*)&sa, sizeof(sa))) {
    perror("bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  //监听 第二个参数控制监听队列大小
  if (-1 == listen(SocketFD, 10)) {
    perror("lister failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  for (;;) {
    // 接受 socket
    int ConnectFD = accept(SocketFD, NULL, NULL);
    if (0 == ConnectFD) {
      perror("accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    int writeSize = 0;
    size_t totalWrite = 0;
    while (totalWrite < sizeof(hello)) {
      writeSize =
          write(ConnectFD, hello + totalWrite, sizeof(hello) - totalWrite);
      if (-1 == writeSize) {
        perror("write failed");
        close(ConnectFD);
        close(SocketFD);
        exit(EXIT_FAILURE);
      }
      totalWrite += writeSize;
    }
    if (-1 == shutdown(ConnectFD, SHUT_RDWR)) {
      perror("shutdown failed");
      close(ConnectFD);
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    close(ConnectFD);
  }
  close(SocketFD);
  return EXIT_SUCCESS;
}
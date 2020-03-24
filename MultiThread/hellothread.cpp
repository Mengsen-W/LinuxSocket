/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-22 10:56:26
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-23 15:32:27
 */

// POSIX threads and C++11 Stdlib
// balance analysis and data
// so use multithread to handle
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

void helloWorld() { std::cout << "hello thread\n"; }

int main() {
  std::thread t(helloWorld);
  std::cout << "hello main thread\n";
  t.join();
  return 0;
}

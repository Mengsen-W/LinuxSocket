/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-24 21:23:37
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-24 21:26:56
 */

// unique_lock example
#include <iostream>  // std::cout
#include <mutex>     // std::mutex, std::unique_lock
#include <thread>    // std::thread

std::mutex mtx;  // mutex for critical section

void print_block(int n, char c) {
  // critical section (exclusive access to std::cout signaled by lifetime of
  // lck):
  std::unique_lock<std::mutex> lck(mtx);
  // dead lock
  // mtx.lock();
  for (int i = 0; i < n; ++i) {
    std::cout << c;
  }
  std::cout << '\n';
}

int main() {
  std::thread th1(print_block, 50, '*');
  std::thread th2(print_block, 50, '$');

  th1.join();
  th2.join();

  return 0;
}
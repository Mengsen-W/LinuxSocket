/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 15:41:48
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 17:09:56
 */

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

static std::mutex mutex;
static std::atomic<bool> ready(false);

void worker(int i) {
  // 自旋锁 一直在判断条件是否满足
  while (!ready) {
    // do nothing but wait
  }
  printf("helloWorld %d\n", i);
  // printf 是线程安全的
  // cout 由多个过程组成线程不安全
  // std::cout << " hello world! " << i << std::endl;
}

int main() {
  const auto threadCount = 4;
  // thread pool
  std::vector<std::thread> pool;
  // start thread
  for (int i = 0; i < threadCount; ++i) {
    pool.emplace_back(worker, i);
  }
  std::this_thread::sleep_for(std::chrono::minutes(1));
  ready = true;

  // thread join
  for (auto& t : pool) {
    if (t.joinable()) t.join();
  }
  std::cout << "byebye " << std::endl;

  return 0;
}

/**
 * 最好是发送信号才处理
 * while 是自旋锁浪费处理机
 * yield 是放弃本次时间片但是由于时间片轮转的原因
 *    可能仍然存在反复调用，使 cpu 自旋，调用和 调度算法有关
 * sleep 和 sleep 时间有关
 *    太多可能浪费 cpu 资源 cpu 空等，太少和 yield差别不大

*/
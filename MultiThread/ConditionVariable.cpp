/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 18:29:43
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 19:54:19
 */

// *一种通信的方法

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Message {
 public:
  const std::string& data() const { return m_data; }
  Message(std::string d = std::string()) : m_data(std::move(d)) {}

 private:
  std::string m_data;
};

static std::mutex mutex;
static std::condition_variable cv;
static std::atomic<bool> ready{false};
static std::atomic<bool> quit{false};
static std::list<Message> globalList;

void worker(int i) {
  while (!ready) {
  }
  while (!quit) {
    std::unique_lock<std::mutex> lock(mutex);
    // 等待条件满足
    // 反复调用它，直到结果为true
    // 加锁进互斥量
    // 以上是一个原子操作
    cv.wait(lock, [] { return quit || !globalList.empty(); });
    // if (quit) return;
    if (globalList.empty()) continue;
    auto iter = globalList.begin();
    globalList.erase(iter);
  }
}

int main() {
  const auto threadCount = 4;
  // 初始化list
  for (int i = 0; i < 10000; ++i) {
    globalList.push_back("this is a test" + std::to_string(i));
  }
  // 创建线程池
  std::vector<std::thread> pool;
  for (int i = 0; i < threadCount; ++i) {
    pool.emplace_back(worker, i);
  }
  // 结束准备
  ready = true;
  // 模拟工作
  for (int i = 0; i < 10000; ++i) {
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      std::lock_guard<std::mutex> lock(mutex);
      globalList.push_back(std::string("second"));
    }
    cv.notify_one();
  }

  // 退出
  while (true) {
    std::lock_guard<std::mutex> lock(mutex);
    if (globalList.empty()) {
      quit = true;
      cv.notify_all();
      break;
    }
  }

  for (auto& t : pool)
    if (t.joinable()) t.join();
  return 0;
}
// 0.44s user 0.00s system 1% cpu 40.004 total

// 我自己的理解
/**
 * 上锁的时候会给wait信号，他不会检查
 * 解锁的时候唤醒wait，然后wait拿锁检查，若可以则继续
 * 不可以马上放弃锁，并把自己线程阻断，直到能拿到锁或是被唤醒，然后检查条件
 */
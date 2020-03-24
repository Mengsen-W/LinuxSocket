/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 16:54:11
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 18:28:02
 */

#include <atomic>
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
static std::atomic<bool> ready{false};
static std::atomic<bool> quit{false};
static std::list<Message> globalList;
static std::atomic<int> totalSize = 0;

void worker(int i) {
  while (!ready) {
  }
  while (!quit) {
    // TODO2 修改
    // 若空则休息1毫秒
    if (globalList.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    Message msg;
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (globalList.empty()) continue;
      auto iter = globalList.begin();
      msg = std::move(*iter);
      globalList.erase(iter);
    }
    totalSize += strlen(msg.data().c_str());
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
    // 等待1毫秒是等待网络 不用也锁住 只有插入的时候要锁一下
    // TODO1. 修改
    // std::lock_guard<std::mutex> lock(mutex);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::lock_guard<std::mutex> lock(mutex);
    globalList.push_back(std::string("second"));
  }

  // 退出
  while (true) {
    std::lock_guard<std::mutex> lock(mutex);
    if (globalList.empty()) {
      quit = true;
      break;
    }
  }

  for (auto& t : pool)
    if (t.joinable()) t.join();
  return 0;
}

/**
 * !! 锁用的太密集而且粒度太粗
 * !! 实际上是一个伪多线程
 * !! 每次都是一个线程在运行其他线程在等待
 * !! 在这个线程中大量时间消耗在等待网络包
 */

// TODO0 初始状态
/**
 * 0.59s user 0.30s system 2% cpu 40.210 total
 */

// TODO1. 结束
/**
 * 45.18s user 34.33s system 196% cpu 40.501 total
 * 修改 TODO1 后自旋锁产生处理机占用变高了
 * 从主线程等待变成子线程自旋
 * 已经不再是一个伪多线程了
 * 但仍然产生自旋消耗过多处理机性能
 */

// TODO2 结束
/**
 * 0.15s user 0.31s system 1% cpu 40.013 total
 * 在拿锁前加判断若空休息1毫秒
 * 减少了大部分自旋等待
 */

/**
 * 0.16s user 0.29s system 1% cpu 40.020 total
 * 0.46s user 0.49s system 1% cpu 40.025 total
 */
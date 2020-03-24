/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 10:16:50
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 12:23:47
 */

#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class Counter {
 public:
  Counter() : m_count{0}, m_totalResource{0} {}
  int count() const { return m_count; }
  int resource() const { return m_totalResource; }
  void addCount() { m_count++; }
  void addResource() { m_totalResource++; }
  int avgResource() {
    if (m_count == 0) {
      return -1;
    }
    int temp = m_totalResource / m_count;
    return temp;
  }

 private:
  int m_count = 0;
  int m_totalResource = 0;
};

static std::vector<int> vec(100000);

static void init_vec(std::vector<int>& vec) {
  for (int i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }
}
static std::mutex p_mutex;

void printStep(Counter& counter) {
  std::cout << "thread id = " << std::this_thread::get_id()
            << " count = " << counter.count()
            << " resource = " << counter.resource()
            << " avgResource = " << counter.avgResource() << '\n';
}

void cycle(Counter& counter) {
  p_mutex.lock();
  counter.addCount();
  counter.addResource();
  printStep(counter);
  p_mutex.unlock();
}

template <typename T>
void realWork(Counter& counter, T& iterBegin, T iterEnd) {
  for (; iterBegin != iterEnd; ++iterBegin) {
    // 在整个for里面其实不是互斥的，两个线程可能会一起进入，导致可能开始循环打印的不是1而是2
    // 继续运行后两个线程就会有一个的位移差就不会出现多次调用,两个线程变成异步了
    // counter.addCount();
    // counter.addResource();
    // printStep(counter);
    cycle(counter);
  }
  return;
}

int main() {
  init_vec(vec);
  Counter counter;
  auto iterBegin = vec.begin();
  auto iterMiddle = vec.begin() + (vec.size() / 2);
  auto iterEnd = vec.end();
  // main thread
  std::thread a([&]() { realWork(counter, iterBegin, iterMiddle); });
  std::thread b([&]() { realWork(counter, iterMiddle, iterEnd); });
  a.join();
  b.join();
  return 0;
}

// 自增是原子的 但调用函数不是原子的
// 有可能两个线程同时进入函数然后自增两倍
// 所以最大是1500
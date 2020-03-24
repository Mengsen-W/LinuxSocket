/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 12:20:24
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 12:28:37
 */
// lock_guard<std::mutex> lock(m_mutex)

// std::lock(a.mutex, b.,mutex...); 把需要的锁用原子操作全部锁住
// std::lock_guard<std::mutex> lockA(a.mutex, std::adopt_lock); 不上锁但是解锁
// std::lock_guard<std::mutex> lockB(b.mutex, std::adopt_lock); 不上锁但是解锁

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

static std::vector<int> vec(10000);

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
  std::lock_guard<std::mutex> lock{p_mutex};
  counter.addCount();
  counter.addResource();
  printStep(counter);
}

template <typename T>
void realWork(Counter& counter, T& iterBegin, T iterEnd) {
  for (; iterBegin != iterEnd; ++iterBegin) {
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

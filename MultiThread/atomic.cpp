/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-24 09:54:05
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-24 10:42:55
 */

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

class Counter {
 public:
  Counter() : m_count{0}, m_totalResource{0} {}
  void addCount() { m_count++; }
  int count() const { return m_count; }
  void addResource(int i) { ++m_totalResource; }
  int avgResource() {
    if (m_count == 0) return -1;
    return m_totalResource / m_count;
  }

 private:
  std::atomic<int> m_count = 0;
  std::atomic<int> m_totalResource = 0;
  // 一些常用的已经预先定义了宏
  // std::atomic_int m_cout
};

int work(int a) {
  // do something
  return a + a;
}

template <typename T>
void realWork(Counter& counter, double& totalvalue, T iterBegin, T iterEnd) {
  for (; iterBegin != iterEnd; ++iterBegin) {
    totalvalue += work(*iterBegin);
    counter.addCount();
    counter.addResource(1);
  }
  return;
}

int main() {
  std::vector<int> vec;
  for (size_t i = 0; i < 10000000; i++) {
    vec.push_back(rand() % 100);
  }

  // singal thread
  double totalvalue;
  Counter counter;
  for (auto v : vec) {
    totalvalue += work(v);
    counter.addCount();
    counter.addResource(1);
  }
  std::cout << "total times = " << counter.count()
            << " avgResource = " << counter.avgResource()
            << " total value = " << totalvalue << std::endl;
  totalvalue = 0;

  Counter counter2;
  // 采用原子操作
  auto iterBegin = vec.begin() + (vec.size() / 3);
  auto iterMiddle = (vec.begin() + (vec.size() / 3 * 2));
  auto end = vec.end();

  // vec.begin() 是一个右值
  realWork(counter2, totalvalue, vec.begin(), iterBegin);  // 主线程
  std::thread a(
      [&counter2, &totalvalue, iterBegin, iterMiddle] {  //第一个子线程
        realWork(counter2, totalvalue, iterBegin, iterMiddle);
      });
  std::thread b([&counter2, &totalvalue, iterMiddle, end] {  // 第二个子线程
    realWork(counter2, totalvalue, iterMiddle, end);
  });

  a.join();
  b.join();
  std::cout << "total times use mulithread = " << counter2.count()
            << " avgResource = " << counter2.avgResource()
            << " totalValue = " << totalvalue << std::endl;

  return 0;
}

// 这里 atomic 将 count 变成原子操作
// 但是对于 work() 内部的操作没有变为原子操作
// 由于过于灵活，使用会有一些不方便，其他的选项也过多
// addCount and addResource is a atomic operator themselves
// but merge for some function is not is atomiced
// so maybe addCount() run twice and addResource run once in time
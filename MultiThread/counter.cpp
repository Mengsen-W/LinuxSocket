/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-23 15:32:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-23 17:00:46
 */

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

class Counter {
 public:
  void addCount() { m_count++; }
  int count() const { return m_count; }
  Counter() : m_count{0} {}

 private:
  int m_count;
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
  }
  return;
}

int main() {
  // singal thread
  std::vector<int> vec;
  for (size_t i = 0; i < 10000000; i++) {
    vec.push_back(rand() % 100);
  }
  double totalvalue;
  Counter counter;
  for (auto v : vec) {
    totalvalue += work(v);
    counter.addCount();
  }
  std::cout << "total times = " << counter.count() << std::endl;
  totalvalue = 0;

  Counter counter2;
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
  std::cout << "total times use mulithread = " << counter2.count() << std::endl;

  return 0;
}

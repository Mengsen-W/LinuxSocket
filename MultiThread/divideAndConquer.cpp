/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-22 16:57:09
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-23 15:40:08
 */
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

double calculate(double v) {
  if (v <= 0) return v;
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  return std::sqrt((v * v + std::sqrt((v - 5) * (v + 2.5)) / 2.0) / v);
}
// for_each
template <typename Iter, typename Fun>
double visitRange(std::thread::id id, Iter iterBegin, Iter iterEnd, Fun func) {
  // 被哪个线程调用会输出哪个线程
  auto curId = std::this_thread::get_id();
  if (id == curId)
    std::cout << curId << " = hello main thread\n";
  else
    std::cout << curId << " = hello work thread\n";
  double v = 0;
  for (auto iter = iterBegin; iter != iterEnd; ++iter) {
    v += func(*iter);
  }
  return v;
}

int main() {
  auto mainThreadId = std::this_thread::get_id();
  std::vector<double> v;
  for (int i = 0; i < 1000; ++i) {
    v.push_back(rand());
  }
  std::cout << v.size() << std::endl;
  double value = 0.0;
  std::cout << mainThreadId << " = hello main thread" << std::endl;
  auto nowc = std::clock();
  for (auto& info : v) {
    value += calculate(info);
  }
  auto finc = std::clock();
  std::cout << "singal thread = " << value << " time = " << (finc - nowc)
            << std::endl;

  // 前半部分
  auto iter = v.begin() + (v.size() / 2);
  // 后半部分
  double anotherv = 0.0;
  auto iterEnd = v.end();

  nowc = std::clock();
  // 主线程
  auto halfv = visitRange(mainThreadId, v.begin(), iter, calculate);
  // 子线程
  std::thread s([&anotherv, mainThreadId, iter, iterEnd]() {
    anotherv = visitRange(mainThreadId, iter, iterEnd, calculate);
  });
  finc = std::clock();

  std::cout << "multithread = " << (halfv + anotherv)
            << " time = " << (finc - nowc) << std::endl;
  auto id = s.get_id();
  std::cout << id << " = hello work thread" << std::endl;
  s.join();

  std::cout << "multithread = " << (halfv + anotherv) << std::endl;
  return 0;
}

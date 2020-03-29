/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-29 20:34:14
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 21:02:32
 * @Description: async
 */

//? async(func, param...) 不另开线程
//? async(flag, func, param...) 另开线程

#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <vector>

template <typename RAIter>
int parallel_sum(RAIter begin, RAIter end) {
  auto length = end - begin;
  if (length < 10000) return std::accumulate(begin, end, 0);

  RAIter mid = begin + length / 2;

  //* 异步非阻塞另开线程
  auto handle = std::async(std::launch::async, parallel_sum<RAIter>, mid, end);

  int sum = parallel_sum(begin, mid);

  return sum + handle.get();
}

int main(int argc, char* argv[]) {
  std::vector<int> v(100000000, 1);
  std::cout << "Ths sum is " << parallel_sum(v.begin(), v.end()) << std::endl;

  return 0;
}
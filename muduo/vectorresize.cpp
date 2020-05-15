/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-15 19:18:49
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-15 19:46:57
 * @Description: vector resize()
 */

#include <iostream>
#include <memory>
#include <vector>

class test {
 public:
  test() { std::cout << "construct" << std::endl; }
  ~test() { std::cout << "destory" << std::endl; }
};

typedef std::vector<std::unique_ptr<test>> testVec;
typedef testVec::value_type testPtr1;
typedef std::unique_ptr<test> testPtr2;

int main() {
  testVec vec;
  // testPtr1 ptr1(new test);
  // testPtr2 ptr2{new test};

  // std::cout << typeid(ptr1).name() << std::endl;
  // std::cout << typeid(ptr2).name() << std::endl;

  for (int i = 0; i < 10; ++i) {
    vec.emplace_back(new test);
  }
  std::cout << "begin erase" << std::endl;
  vec.erase(vec.begin() + 8, vec.end());
  std::cout << "end erase" << std::endl;

  std::cout << "begin resize" << std::endl;
  vec.resize(5);
  std::cout << "end resize" << std::endl;

  std::cout << "begin resize" << std::endl;
  vec.reserve(3);
  // just advice
  // if member < reserve it can reserve
  // else no affected
  std::cout << "end resize" << std::endl;

  std::cout << "begin reset" << std::endl;
  vec[4].reset(nullptr);
  std::cout << "reset reset" << std::endl;

  std::cout << "clear vec" << std::endl;
  vec.clear();
  std::cout << "end clear vec"<< std::endl;
  return 0;
}
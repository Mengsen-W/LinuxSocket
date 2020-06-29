/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-29 15:46:16
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-29 16:26:06
 * @Description: 测试 vector 析构成员
 */

#include <iostream>
#include <memory>
#include <vector>

class T {
 public:
  T(int a) : _a(a) {}
  ~T() { std::cout << _a << "destruction" << std::endl; }
  void put() const { std::cout << _a << std::endl; }

 private:
  int _a;
};

static std::vector<int> vec;
static std::vector<T> vecT;
static std::vector<T*> vecTPtr;
static std::vector<std::shared_ptr<T>> vecS;

void testint() {
  for (int i = 0; i < 5; ++i) {
    vec.emplace_back(i);
  }
  std::cout << "exit test()" << std::endl;
  return;
}

void testclass() {
  for (int i = 0; i < 5; ++i) {
    vecT.push_back(T(i));
  }
  std::cout << "des------------------------" << std::endl;

  // 用户显式调用析构函数的时候，只是单纯执行析构函数内的语句，不会释放栈内存，也不会摧毁对象。
  vecT.front().~T();

  return;
}

void testclassptr() {
  for (int i = 0; i < 5; ++i) {
    vecTPtr.emplace_back(new T(i));
  }

  for (T* num : vecTPtr) {
    num->put();
  }
  delete vecTPtr.front();
  return;
}

void testshared() {
  for (int i = 0; i < 5; ++i) {
    vecS.emplace_back(std::make_shared<T>(i));
  }
  return;
}

int main() {
  // testint();

  // for (int num : vec) {
  //   std::cout << num << std::endl;
  // }

  testclass();

  for (T& num : vecT) {
    num.put();
  }

  // testclassptr();

  // for (T* num : vecTPtr) {
  //   //! 内存错误其中某一个指针指向了已删除区域，产生内存错误
  //   num->put();
  //   delete num;
  // }

  return 0;
}
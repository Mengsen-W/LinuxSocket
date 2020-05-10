/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-10 21:54:00
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-10 22:47:14
 * @Description: SFINAE
 */

#include <cstdint>
#include <cstdio>
#include <iostream>

template <typename T>
struct has_no_destroy {
  template <typename C>
  static int32_t test(...);
  template <typename C>
  static char test(decltype(&C::no_destroy));
  // 这里可以算作传入了一个函数指针
  // 或者可以理解为一个成员变量指针

  // 传0作为参数是因为0可以转化NULL
  // 这个空值其实类型是任意的
  // 因为0这两种情况都可以用作参数（即，两个重载test
  // 用于成员指针（视为空指针）和可变参数...
  // 在函数声明中，函数的参数必须是类型 为函数指定参数的类型。
  // &C::Passengers不是类型 (在其预期用途的上下文中)
  // decltype()会自动获取其参数的类型
  // 如果第一个满足 0 会被当作一个第一个函数类型的指针

  // 这里函数必须要加模版
  // 因为这个特性只是相对于模版来说的
  // 如果函数不加模版，那么在构造类的过程中就会出粗
  // 因为实际上类中的函数是不会晚绑定的
  // 会在类确定的时候进行全部的特例化
  // 在特例化的过程中就会出错

  // 而对于函数模版来说
  // 只有在特例化调用的过程中才会绑定
  // 也就是类构造的时候没有确定

  // 只有这样才能在最终确定类型并且不会出错
  const static bool value = sizeof(test<T>(0)) == 1;
};
// 其作用就是用来判断是否有 no_destroy 函数

struct A {};

struct B {
  void no_destroy() {}
};
struct C {
  int no_destroy;
};

struct D : B {};

void testNoDestroy() {
  printf("%d\n", has_no_destroy<A>::value);
  printf("%d\n", has_no_destroy<B>::value);
  printf("%d\n", has_no_destroy<C>::value);
  printf("%d\n", has_no_destroy<D>::value);
}

void fun(double a, double b) {
  std::cout << "fun(double,double)" << std::endl;
  return;
}
void fun(int a, int b) {
  std::cout << "fun(int,int)" << std::endl;
  return;
}
int main() {
  // testNoDestroy();
  /*   fun(1, 1);
    fun(1.1, 1);  // 二义性 */
  return 0;
}
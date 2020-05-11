/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-10 18:38:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-11 11:08:56
 * @Description: 单例类
 */

#include <pthread.h>

#include <cassert>
#include <iostream>

template <typename T>
struct has_no_destroy {
  template <typename C>
  static char test(decltype(&C::no_destroy));
  // 使用 SFINAE 技术判断是否有 T::no_destory
  // 如果没有不会报错而是继续执行直到找到一个合适的或者找不到报错
  template <typename C>
  static int32_t test(...);
  const static bool value = sizeof(test<T>(0)) == 1;
  // const 必须在声明时初始化，或者在构造函数初始化列表中初始化
};

template <typename T>
class Singleton {
 public:
  Singleton() = delete;  // 没有构造关于单例类的任何实例
  ~Singleton() = delete;

  // 已经禁止构造了就没有任何实例在外面
  // 后面三个没有必要写出来
  /*   Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete; */

  static T& instance() {
    pthread_once(&ponce_, &Singleton::init);  // 保证init在这个进程中只出现一次
    assert(value_ != nullptr);
    // 在init内部已经完成了 value_ 的赋值
    // 在其后的任何情况下都 value_ 的值都不能为 nullptr
    // 如果其值为假（即为0），那么它先向stderr打印一条出错信息
    // 然后通过调用abort 来终止程序运行
    return *value_;
  }

 private:
  static void init() {
    std::cout << "coming init()" << std::endl;

    value_ = new T();  // 调用的实际上是T的构造函数

    if (!detail::has_no_destroy<T>::value) {
      ::atexit(destroy);
    }
  }

  static void destroy() {
    // 这是一种不完全类型检测
    // T如果不是不完全类型那么sizeof（T）
    // 就应该是type_must_be_complete[-1],数组是不能为负数的，所以就会报错
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];

    // 实际定义一个数组
    //! warring T_must_be_complete_type’ locally defined but not used
    T_must_be_complete_type dummy;
    //! warning: unused variable ‘dummy’
    (void)dummy;

    delete value_;
    value_ = NULL;
  }

 private:
  static pthread_once_t ponce_;
  static T* value_;
  // 因为没有具体的实例所有这里 const int 并没有构造出，所以不会报错
  static const int i;
};

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
template <typename T>
T* Singleton<T>::value_ = nullptr;

class Foo {
 public:
  static int print() { return i; }

 private:
  static const int i;
  // static 这里不初始化在编译期是可行的
  // 但是一旦用了这个值因其未初始化故会链接错误
  // 也就是说实际上 static 是不属于任何一方的
  // 当只有 const 的时候 在编译期一定会看到
};

int main() {
  // int& sing2 = Singleton<int>::instance();
  // sing2 = 1;
  // int& sing3 = Singleton<int>::instance();
  // std::cout << sing2 << "\t" << sing3 << '\t' << std::endl;
  // Foo f;
  // std::cout << f.print() << std::endl;
  // std::cout << Foo::print() << std::endl;

  return 0;
}
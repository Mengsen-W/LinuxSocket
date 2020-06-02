#include <iostream>
#include <memory>
#include <vector>

class A;

class B {
 public:
  B() : _ptr(nullptr) {}
  B(std::shared_ptr<A> ptr) : _ptr(ptr) {}
  void test() { std::cout << "B test" << std::endl; }

 private:
  std::shared_ptr<A> _ptr;
};

class A : std::enable_shared_from_this<A> {
 public:
  void creat() {
    vec_ptr = std::make_shared<std::vector<std::shared_ptr<B>>>();
    std::shared_ptr<B> ptr = std::make_shared<B>(shared_from_this());
    ptr->test();
    vec_ptr->emplace_back(ptr);
    vec_ptr->at(0)->test();
    // vec_ptr->emplace_back(std::make_shared<B>(shared_from_this()));
    // vec_ptr->at(0)->test();
  }
  void test() { std::cout << "A test" << std::endl; }

 private:
  std::shared_ptr<std::vector<std::shared_ptr<B>>> vec_ptr;
};

void foo(std::shared_ptr<A> a_ptr) {
  a_ptr->creat();
  // ptr->test();
}

struct C {
  int* data;
};

int fun() {
  std::shared_ptr<int> p1;
  std::shared_ptr<int> p2(nullptr);
  std::shared_ptr<int> p3(new int);
  std::shared_ptr<int> p4(new int, std::default_delete<int>());
  std::shared_ptr<int> p5(
      new int, [](int* p) { delete p; }, std::allocator<int>());
  std::cout << "p5: " << p5.use_count() << '\n';
  std::shared_ptr<int> p6(p5);
  std::cout << "p5: " << p5.use_count() << '\n';
  std::cout << "p6: " << p6.use_count() << '\n';
  std::shared_ptr<int> p7(std::move(p6));
  std::cout << "p5: " << p5.use_count() << '\n';
  std::cout << "p6: " << p6.use_count() << '\n';
  std::cout << "p7: " << p7.use_count() << '\n';
  std::shared_ptr<int> p8(std::unique_ptr<int>(new int));
  std::shared_ptr<C> obj(new C);
  std::shared_ptr<int> p9(obj, obj->data);

  std::cout << "use_count:\n";
  std::cout << "p1: " << p1.use_count() << '\n';
  std::cout << "p2: " << p2.use_count() << '\n';
  std::cout << "p3: " << p3.use_count() << '\n';
  std::cout << "p4: " << p4.use_count() << '\n';
  std::cout << "p5: " << p5.use_count() << '\n';
  std::cout << "p6: " << p6.use_count() << '\n';
  std::cout << "p7: " << p7.use_count() << '\n';
  std::cout << "p8: " << p8.use_count() << '\n';
  std::cout << "p9: " << p9.use_count() << '\n';
  return 0;
}

int main() {
  // 用 make_shared 初始化
  std::shared_ptr<A> a_ptr = std::make_shared<A>();
  a_ptr->test();
  foo(a_ptr);

  // fun();
  return 0;
}
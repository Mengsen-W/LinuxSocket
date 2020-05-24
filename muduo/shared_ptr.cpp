/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-24 10:04:40
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-24 10:12:02
 */

#include <iostream>
#include <memory>

class Object : public std::enable_shared_from_this<Object> {
 public:
  typedef std::shared_ptr<Object> ptr;
  // constructor do not use shared_from_this()
  Object() { Object::ptr p = shared_from_this(); }

  ~Object() = default;
};

void test(std::shared_ptr<int>& i) {
  // no reference just pass value // 2
  std::cout << i.use_count() << std::endl;  // 1
  return;
}

int main() {
  // std::shared_ptr<int> s = std::make_shared<int>(10);
  // std::cout << s.use_count() << std::endl;  // 1
  // test(s);
  // std::cout << s.use_count() << std::endl;  // 1

  Object o;

  return 0;
}
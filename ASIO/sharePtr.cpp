/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-26 21:07:19
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 07:35:59
 */

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void print(const boost::system::error_code &e,
           std::shared_ptr<boost::asio::deadline_timer> t, int *count) {
  if (*count < 1) {
    std::cout << t << "t use_count in print = " << t.use_count() << std::endl;
    t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
    t->async_wait([t, count](const auto &error) { print(error, t, count); });
    ++(*count);
  }
  std::cout << t << "t use_count in print = " << t.use_count() << std::endl;
  return;
}

std::shared_ptr<boost::asio::deadline_timer> registerPrint(
    boost::asio::io_service &io, int *count) {
  std::shared_ptr<boost::asio::deadline_timer> t =
      std::make_shared<boost::asio::deadline_timer>(
          io, boost::posix_time::seconds(1));
  std::cout << t << "t use_count in registerPrint = " << t.use_count()
            << std::endl;
  t->async_wait(
      [t, count](const boost::system::error_code &e) { print(e, t, count); });
  std::cout << t << "t use_count in registerPrint = " << t.use_count()
            << std::endl;
  return t;
}

int main() {
  boost::asio::io_service io;
  std::vector<int> v{0, 1, 2, 3, 4};
  for (int i = 0; i < 1; ++i) {
    std::shared_ptr<boost::asio::deadline_timer> t = registerPrint(io, &v[i]);
    std::cout << t << "t use_count in main = " << t.use_count() << std::endl;
  }

  io.run();
  return 0;
}
/*
 * 事件传入会增加一次引用计数
 * 也就是事件本身会那都一个引用计数
 * 使用智能指针可以保证没问题
 * 创造（1） -> 事件（2） -> 出registerPrint(1) -> 拷贝给main(2) ->
 * 退出作用域(1) -> 执行事件(1) -> 事件拷贝给 print (2)
 * 进入 print 内部事件(3) -> 出print(2) -> 析构事件(1)
 * 进 print 事件(2)
 * -> 没有继续事件了(2) ->出 print (1) -> 析构事件(0)
 */
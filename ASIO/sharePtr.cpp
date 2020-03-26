/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-26 21:07:19
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-26 22:08:01
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
    // std::cout << *count << std::endl;

    std::cout << t << "t use_count in print = " << t.use_count() << std::endl;
    t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
    t->async_wait([t, count](const auto &error) { print(error, t, count); });
    ++(*count);
    // std::cout << "finished register event\t" << *count << std::endl;
    // std::cout << e << std::endl;
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
 * 创造（1） -> 事件（2） -> 出registerPrint(1) -> 拷贝给main(2)
 * -> 进 print(2) 事件 -> 事件(3) -> 出print(2) ->
 * 进 print(2) 事件 -> 没有继续事件了(2) ->出 print (1) ->
 * 出主函数(0)
 */
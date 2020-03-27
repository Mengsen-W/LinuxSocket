/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-26 17:15:38
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 07:54:27
 */

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <iostream>
#include <memory>
#include <string>

void print(const boost::system::error_code &e,
           std::shared_ptr<boost::asio::deadline_timer> t, int *count) {
  if (*count < 3) {
    std::cout << *count << std::endl;

    // 重新制定下一次唤醒的时间
    // 加入下一次唤醒时的任务
    // 注册的事件是调用函数
    t->async_wait([t, count](const auto &error) { print(error, t, count); });
    t->expires_at(t->expires_at() + boost::posix_time::seconds(0));

    ++(*count);
    std::cout << "finished register event\t" << *count << std::endl;
    std::cout << e << std::endl;
  }
  return;
}

int main() {
  boost::asio::io_service io;

  int count = 0;
  std::shared_ptr<boost::asio::deadline_timer> t =
      std::make_shared<boost::asio::deadline_timer>(
          io, boost::posix_time::seconds(5));
  t->async_wait([t, &count](const auto &error) { print(error, t, &count); });

  // 只有run的时候程序才开始运行，其他都是在注册事件
  // 异步操作结束前堵塞，所有异步结束后返回
  io.run();

  std::cout << "Final count is = " << count << std::endl;
  return 0;
}

//! 这就是面向事件非阻塞异步的逻辑
//! 非阻塞是注册完事件后继续执行
//! 异步是由io唤醒
//! 面向事件是说明没有事件直接返回
//! 若不设置timer，则马上返回
//! 实际上还是单线程的，当函数执行完后才会查看或是调用事件

//! 在注册事件之前就设定定时器否则定时器无法使用，也就是每次事件完成就会释放计时器
//! 事件完成后会把计时器清零，虽然后面调用了计时器，但是事件完成后会释放计时器，
//! 顺序是先推入事件，设置计时器，事件完成，释放计时器
//! 若未设置定时器，会返回125错误码,也即是返回错误，因为按照boost里面必须设置计时器
//! 125 operation_aborted
//! 遇到 125 会调用系统终止，马上调用回调函数
//! 本身不会在编译期报错的原因，是因为使用了错误码处理错误
//! 但是除非是手动给否则错误就会产生
//! 利用了注册事件的时候构造函数是否用了默认值

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-26 16:40:56
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-26 21:10:59
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <iostream>

void print(const boost::system::error_code &, boost::asio::deadline_timer *t,
           int *count) {
  if (*count < 5) {
    std::cout << *count << std::endl;
    ++(*count);

    // 重新制定下一次失效的时间
    t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
    t->async_wait(
        boost::bind(print, boost::asio::placeholders::error, t, count));
  }
  return;
}

int main() {
  boost::asio::io_service io;

  int count = 0;
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
  t.async_wait(
      boost::bind(print, boost::asio::placeholders::error, &t, &count));

  io.run();

  std::cout << "Final count is = " << count << std::endl;
  return 0;
}

// maybe this is a noneblock asynchronous
/**
 * 0
 * 1
 * 2
 * 3
 * 4
 * Final count is = 5
 * ./asio_bind.out  0.00s user 0.00s system 0% cpu 6.005 total
 */
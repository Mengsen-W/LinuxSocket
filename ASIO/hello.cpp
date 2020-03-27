/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-26 15:40:25
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 07:54:26
 * @Description: Hello ASIO
 */

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
// #include <boost/asio/steady_timer.hpp>
#include <iostream>

void callback(const boost::system::error_code&) {
  std::cout << "Hello Asynchronous" << std::endl;
  return;
}
void callback2(const boost::system::error_code&) {
  std::cout << "Hello Asynchronous2" << std::endl;
  return;
}

int main() {
  // register io_service for ASIO
  boost::asio::io_service io;

  // timer
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));
  //!! synchronization block model
  // t.wait();
  // print after 5 seconds
  // std::cout << " Hello, ASIO " << std::endl;
  boost::asio::deadline_timer t2(io, boost::posix_time::seconds(2));

  //!! asynchronous noneblock model
  t.async_wait(callback);
  t2.async_wait(callback2);
  std::cout << "start run" << std::endl;
  // io.run will block unfinished
  io.run();
  std::cout << "finish run" << std::endl;

  return 0;
}

/**
 * 0.00s user 0.00s system 0% cpu 5.011 total
 */
/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 07:29:09
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 08:07:25
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <memory>
#include <vector>

class printer {
 public:
  printer(boost::asio::io_service &io)
      : timer_(io, boost::posix_time::seconds(1)), count_(0) {
    timer_.async_wait(boost::bind(&printer::print, this));
  }
  ~printer() { std::cout << "Final count is = " << count_ << std::endl; }
  void print() {
    if (count_ < 5) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
      timer_.async_wait(boost::bind(&printer::print, this));
    }
  }

 private:
  int count_;
  boost::asio::deadline_timer timer_;
};

int main() {
  boost::asio::io_service io;
  printer p{io};
  io.run();
  return 0;
}

/*
 * bind 的 的一个参数希望是一个函数指针，第二个参数是调用者指针
 * bind -> function<void(const boost::system::error_code &)>
 */
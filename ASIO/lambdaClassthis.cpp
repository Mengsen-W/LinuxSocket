/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 07:50:50
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 08:30:31
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
    timer_.async_wait(
        [this](const boost::system::error_code &) { this->print(); });
  }
  ~printer() {
    //! 加不加效果一样
    //! 都不可以
    // timer_.cancel();
    std::cout << "Final count is = " << count_ << std::endl;
  }
  void print() {
    if (count_ < 5) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
      timer_.async_wait(
          [this](const boost::system::error_code &) { this->print(); });
    }
  }

 private:
  int count_;
  boost::asio::deadline_timer timer_;
};

int main() {
  boost::asio::io_service io;
  // { printer p{io}; }
  //! stack-use-after-scope
  printer p{io};
  io.run();
  return 0;
}

/*
 * 调用析构的先于事件执行
 * 所以事件执行的时候类已经析构了
 * cancel的作用是执行完本次事件若还有下次事件则把之后的取消
 * 这里执行一次事件的时候下一个事件还没有注册,而且类已经析构了
 * 继续执行的做法已经内存越界了
 * 但是内存可能还没有清空
 * 导致有些时候是可以的
 */
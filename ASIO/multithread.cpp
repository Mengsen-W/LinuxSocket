/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 08:46:15
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 09:47:02
 */

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <thread>

class Printer {
 public:
  Printer(boost::asio::io_service &io)
      : strand_(io),
        timer1_(io, boost::posix_time::seconds(1)),
        timer2_(io, boost::posix_time::seconds(1)),
        count_(0) {
    timer1_.async_wait(strand_.wrap(
        [this](const boost::system::error_code &) { this->print1(); }));
    timer2_.async_wait(strand_.wrap(
        [this](const boost::system::error_code &) { this->print2(); }));
  }
  ~Printer() { std::cout << "Finish count is = " << count_ << std::endl; }

  void print1() {
    if (count_ < 10) {
      std::cout << "Timer 1 = " << count_ << std::endl;
      std::cout << "Thread id of Timer 1 = " << std::this_thread::get_id()
                << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expires_at() + boost::posix_time::seconds(1));
      timer1_.async_wait(strand_.wrap(
          [this](const boost::system::error_code &) { this->print1(); }));
    }
  }
  void print2() {
    if (count_ < 10) {
      std::cout << "Timer 2 = " << count_ << std::endl;
      std::cout << "Thread id of Timer 2 = " << std::this_thread::get_id()
                << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expires_at() + boost::posix_time::seconds(1));
      timer2_.async_wait(strand_.wrap(
          [this](const boost::system::error_code &) { this->print2(); }));
    }
  }

 private:
  boost::asio::io_service::strand strand_;
  boost::asio::deadline_timer timer1_;
  boost::asio::deadline_timer timer2_;
  int count_;
};

int main() {
  boost::asio::io_service io;
  Printer p{io};
  //! 这个线程里面跑的是 io_service 注册的事件
  //! 当 io_service 里面没有注册事件或者产生异常也会返回
  std::thread t([&io]() {
    std::cout << "start T1 run" << std::endl;
    io.run();
    std::cout << "start T1 stop" << std::endl;
  });

  std::cout << "start T0 run" << std::endl;
  io.run();
  std::cout << "stop T0 stop" << std::endl;

  t.join();

  return 0;
}

//! 若事件注册在事件开始运行前, 事件板会直接返回并把状态置为 stopped
//! 一个线程里面的 io_service 状态的改变, 意味着全部线程的同一 io_service 改变
//! io的状态不能由多次 io.run() 来改变 ,而要使用 io.reset() 来改变
//! 保证回调事件在多线程的顺序 使用 strand.wrap()
//! 使事件的完成成为一个原子操作,一个线程在跑一个事件的时候别的线程不能访问这个事件

/*
 * start T1 runstart T0 run
 * Timer 1 = 0
 * Thread id of Timer 1 = 140698120615680
 * Timer 2 = 1
 * Thread id of Timer 2 = 140698120615680
 * Timer 1 = 2
 * Thread id of Timer 1 = 140698211222400
 * Timer 2 = 3
 * Thread id of Timer 2 = 140698211222400
 * Timer 1 = 4
 * Thread id of Timer 1 = 140698120615680
 * Timer 2 = 5
 * Thread id of Timer 2 = 140698120615680
 * Timer 1 = 6
 * Thread id of Timer 1 = 140698211222400
 * Timer 2 = 7
 * Thread id of Timer 2 = 140698211222400
 * Timer 1 = 8
 * Thread id of Timer 1 = 140698211222400
 * Timer 2 = 9
 * Thread id of Timer 2 = 140698211222400
 * start T1 stop
 * stop T0 stop
 * Finish count is = 10
 * ./multithread.out  0.01s user 0.00s system 0% cpu 6.014 total
 */
/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 17:09:38
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 18:56:10
 * @Description: Echo Service
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
 public:
  session(tcp::socket socket) : socket_(std::move(socket)) {}
  // 不能把start放在构造里面
  //! start里面有 do_read() 里面有要求自身指针，但是自身还没有构造完成
  void start() { do_read(); }

 private:
  void do_read() {
    //! 两次引用次数 一次数 self 一次是构造lambda类，当两个全结束后才能析构
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            [this, self](const boost::system::error_code error,
                                         std::size_t length) {
                              if (!error) do_write(length);
                            });
  }
  void do_write(std::size_t length) {
    auto self(shared_from_this());
    socket_.async_write_some(boost::asio::buffer(data_, max_length),
                             [this, self](const boost::system::error_code error,
                                          std::size_t length) {
                               if (!error) do_read();
                             });
  }
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server {
 public:
  server(boost::asio::io_service& io_service, short port)
      : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        socket_(io_service) {}
  void start() {
    std::cout << "Start Accept" << std::endl;
    do_accept();
  }

 private:
  void do_accept() {
    acceptor_.async_accept(socket_, [this](auto& e) {
      // 清理
      if (!e) std::make_shared<session>(std::move(socket_))->start();
      do_accept();
    });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    boost::asio::io_service io_service;
    server s(io_service, std::atoi(argv[1]));
    s.start();
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}

//! 传 self
//! 的原因是等到事件即lambda完成之后才计数为零，也就是说不能先于事件完成就计数为零
//! move 两次的原因时 首先可以清理socket 第二 socket 不支持复制
//! 每个socket都是独一无二的,所以尽量使用移动

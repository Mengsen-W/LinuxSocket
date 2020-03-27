/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 11:39:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 11:41:38
 * @Description: Asynchronous non-block Service
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

std::string make_daytime_string() {
  std::time_t now = time(nullptr);
  return std::ctime(&now);
}

class tcp_connect : public std::enable_shared_from_this<tcp_connect> {
 public:
  using pointer = std::shared_ptr<tcp_connect>;

  static pointer create(boost::asio::io_service& io_service) {
    //! 用不了 make_shared 是一个函数，因为模板需要实例化，会调用一个构造函数
    // return std::make_shared<tcp_connect>(io_service);
    return pointer(new tcp_connect(io_service));
  }

  tcp::socket& socket() { return socket_; }

  void start() {
    message_ = make_daytime_string();

    auto self = shared_from_this();
    //! 尽量不要使用引用方式传递智能指针
    //! 使用 shared_ptr 可以防止调用完这个函数后self失效，尤其对于单例模式
    std::cout << "start write" << std::endl;
    boost::asio::async_write(
        socket_, boost::asio::buffer(message_),
        [self = std::move(self)](const boost::system::error_code& error,
                                 size_t bytes_transferred) {
          self->handle_write(error, bytes_transferred);
        });
  }

 private:
  tcp_connect(boost::asio::io_service& io_service) : socket_(io_service) {}
  void handle_write(const boost::system::error_code& error,
                    size_t bytes_transferred) {}
  tcp::socket socket_;
  std::string message_;
};

class tcp_server {
 public:
  tcp_server(boost::asio::io_service& io_service)
      : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)) {
    start_accept();
  }

 private:
  void start_accept() {
    tcp_connect::pointer new_connect =
        tcp_connect::create(acceptor_.get_io_service());
    std::cout << "start accept " << std::endl;
    acceptor_.async_accept(
        new_connect->socket(),
        [this, new_connect](const boost::system::error_code& error) {
          this->handle_accept(new_connect, error);
        });
  }

  void handle_accept(tcp_connect::pointer new_connect,
                     const boost::system::error_code& error) {
    if (!error) {
      new_connect->start();
    }
    start_accept();
  }
  tcp::acceptor acceptor_;
};

int main() {
  try {
    boost::asio::io_service io_service;
    tcp_server server(io_service);
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-28 10:03:15
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 16:32:08
 * @Description: chat client
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <thread>

#include "chat_message.h"

using boost::asio::ip::tcp;

using chat_message_queue = std::deque<chat_message>;

class chat_client {
 public:
  chat_client(boost::asio::io_service& io_service,
              tcp::resolver::iterator endpoint_iterator)
      : io_service_(io_service), socket_(io_service) {
    do_connect(endpoint_iterator);
  }

  void write(const chat_message& msg) {
    //! 在主线程调用后，放入其他线程调用这个事件
    io_service_.post([this, msg]() {
      bool write_in_progress = !write_msgs_.empty();
      write_msgs_.push_back(msg);
      if (!write_in_progress) {
        std::cout << "---- do write ----" << std::endl;
        do_write();
      }
    });
  }

  void close() {
    io_service_.post([this]() { socket_.close(); });
  }

 private:
  void do_connect(tcp::resolver::iterator endpoint_iterator) {
    boost::asio::async_connect(
        socket_, endpoint_iterator,
        [this](const boost::system::error_code& e, tcp::resolver::iterator) {
          if (!e) {
            do_read_header();
          }
        });
  }

  void do_read_header() {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](const boost::system::error_code& e, std::size_t /*length*/) {
          if (!e && read_msg_.decode_header()) {
            std::cout << "---- read header success ----" << std::endl;
            do_read_body();
          } else {
            //! 在其他线程调用不用post
            std::cout << "---- read header false ----" << e << std::endl;
            socket_.close();
          }
        });
  }

  void do_read_body() {
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](const boost::system::error_code& e, std::size_t /*length*/) {
          if (!e) {
            std::cout << "---- read body success ----" << std::endl;
            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << "\n";
            do_read_header();
          } else {
            std::cout << "---- read body false ----" << std::endl;
            socket_.close();
          }
        });
  }

  void do_write() {
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front().data(),
                            write_msgs_.front().length()),
        [this](const boost::system::error_code& e, std::size_t /*length*/) {
          if (!e) {
            std::cout << "---- do_write success ----" << std::endl;
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) do_write();
          } else {
            std::cout << "---- do_write false ----" << std::endl;
            socket_.close();
          }
        });
  }

  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }
    std::cout << "Client Start ..." << std::endl;
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
    chat_client c(io_service, endpoint_iterator);

    std::thread t([&io_service]() { io_service.run(); });

    char line[chat_message::max_body_length + 1];
    while (std::cin.getline(line, chat_message::max_body_length + 1)) {
      chat_message msg;
      msg.body_length(std::strlen(line));
      std::memcpy(msg.body(), line, msg.body_length());
      msg.encode_header();
      std::cout << "----write----" << std::endl;
      c.write(msg);
    }

    c.close();
    t.join();

    // 若置换顺序则子线程等待关闭，主线程等待子线
    // 但是子线程的关闭要等待子线程返回，产生死锁
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}

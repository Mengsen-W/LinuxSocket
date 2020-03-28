/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-28 10:03:15
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 11:13:57
 * @Description: chat client
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <deque>
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
      if (!write_in_progress) do_write();
    });
  }

  void close() {
    io_service_.post([this]() { socket_.close(); });
  }

 private:
  void do_connect(tcp::resolver::iterator endpoint_iterator) {
    boost::asio::async_connect(socket_, endpoint_iterator,
                               [this](boost::system::error_code& e) {
                                 if (!e) do_read_header();
                               });
  }

  void do_read_header() {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](boost::system::error_code& e, std::size_t) {
          if (!e && read_msg_.decode_header())
            do_read_body();
          else
            //! 在其他线程调用不用post
            socket_.close();
        });
  }

  void do_read_body() {
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](boost::system::error_code& e, std::size_t) {
          if (!e) {
            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << std::endl;
            do_read_header();
          } else
            socket_.close();
        });
  }

  void do_write() {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(write_msgs_.front().data(),
                                                 write_msgs_.front().length()),
                             [this](boost::system::error_code& e, std::size_t) {
                               if (!e) {
                                 write_msgs_.pop_front();
                                 if (!write_msgs_.empty())
                                   do_write();
                                 else
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

      boost::asio::io_service io_service;

      tcp::resolver resolver(io_service);
      tcp::resolver::iterator endpoint_iterator =
          resolver.resolve({argv[1], argv[2]});
      chat_client c(io_service, endpoint_iterator);

      std::thread t([&io_service]() { io_service.run(); });

      char line[chat_message::max_body_length + 1];
      while (std::cin.getline(line, chat_message::max_body_length + 1)) {
        chat_message msg;
        msg.body_length(std::strlen(line));
        std::memcpy(msg.body(), line, msg.body_length());
        msg.encode_header();
        c.write(msg);
      }

      c.close();
      t.join();
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-28 10:03:15
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 11:34:56
 * @Description: chat client by json and multithread
 */

#include <boost/asio.hpp>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "JSONObj.h"
#include "chat_message.h"
#include "serialization.h"
#include "structHeader.h"

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
            if (read_msg_.type() == 3) {
              std::stringstream ss(
                  std::string(read_msg_.body(),
                              read_msg_.body() + read_msg_.body_length()));
              ptree tree;
              boost::property_tree::read_json(ss, tree);
              std::cout << "client: ";
              std::cout << tree.get<std::string>("name");
              std::cout << " says: ";
              std::cout << tree.get<std::string>("information") << '\n';
            }
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
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) do_write();
          } else {
            socket_.close();
          }
        });
  }

  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  boost::asio::io_service m_strand;
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }
    std::cout << "Client Start ..." << std::endl;
    boost::asio::io_service io_service;
    std::vector<std::unique_ptr<chat_client>> clientGroup;

    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
    for (int i = 0; i < 10; ++i) {
      clientGroup.emplace_back(
          std::make_unique<chat_client>(io_service, endpoint_iterator));
    }

    std::thread t([&io_service]() { io_service.run(); });

    char line[chat_message::max_body_length + 1];
    while (std::cin.getline(line, chat_message::max_body_length + 1)) {
      chat_message msg;
      int type = 0;
      std::string input(line, line + std::strlen(line));
      std::string output;
      if (parseMessage(input, &type, output)) {
        msg.setMessage(type, output.data(), output.size());
        std::cout << "send size = " << output.size() << std::endl;
        for (int i = 0; i < 100000; i++) {
          for (auto& v : clientGroup) {
            v->write(msg);
          }
        }
      }
    }

    for (auto& v : clientGroup) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
      v->close();
    }
    t.join();

    // 若置换顺序则子线程等待关闭，主线程等待子线
    // 但是子线程的关闭要等待子线程返回，产生死锁
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}

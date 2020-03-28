/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 21:40:36
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 10:03:42
 * @Description: chat server
 */

/*
 * chat_participant: 虚基类
 * chat_room: 控制交流队列和房间的出入
 * chat_session: 读写数据
 * chat_server: 监听分发socket
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include "chat_message.h"

using boost::asio::ip::tcp;
using chat_message_queue = std::deque<chat_message>;

class chat_participant {
 public:
  using pointer = std::shared_ptr<chat_participant>;
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message& msg) = 0;
};

using chat_participant_ptr = std::shared_ptr<chat_participant>;

class chat_room {
 public:
  void join(chat_participant_ptr participant) {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_) participant->deliver(msg);
  }

  void leave(chat_participant_ptr participant) {
    participants_.erase(participant);
  }

  void deliver(const chat_message& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();

    for (auto& participant : participants_) participant->deliver(msg);
  }

 private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

class chat_session : public std::enable_shared_from_this<chat_session>,
                     public chat_participant {
 public:
  chat_session(tcp::socket socket, chat_room& room)
      : socket_(std::move(socket)), room_(room) {}

  void start() {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg) {
    // first false
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    // just first come in this
    //! 防止重复调用
    if (!write_in_progress) do_write();
  }

 private:
  void do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](boost::system::error_code& e, std::size_t length) {
          if (!e && read_msg_.decode_header()) {
            do_read_body();
          } else {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_read_body() {
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](boost::system::error_code& error, std::size_t length) {
          if (!error) {
            room_.deliver(read_msg_);
            do_read_header();
          } else
            room_.leave(shared_from_this());
        });
  }

  void do_write() {
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front().data(),
                            write_msgs_.front().length()),
        [this, self](boost::system::error_code& error, std::size_t* length) {
          if (!error) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) do_write();
          } else
            room_.leave(shared_from_this());
        });
  }
  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

class chat_server {
 public:
  chat_server(boost::asio::io_service& io_service,
              const tcp::endpoint& endpoint)
      : acceptor_(io_service, endpoint), socket_(io_service) {
    do_accept();
  }

 private:
  void do_accept() {
    acceptor_.async_accept(socket_, [this](boost::system::error_code& error) {
      if (!error)
        std::make_shared<chat_session>(std::move(socket_), room_)->start();
      do_accept();
    });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
  chat_room room_;
};

int main(int argc, char* argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_service io_service;
    std::list<chat_server> servers;
    for (int i = 0; i < argc; ++i) {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_service, endpoint);
    }

    io_service.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
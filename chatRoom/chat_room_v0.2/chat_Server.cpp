/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 21:40:36
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 19:31:42
 * @Description: chat server
 */

/*
//  * chat_participant: 虚基类
 * chat_room: 控制交流队列和房间的出入
 * chat_session: 读写数据
 * chat_server: 监听分发socket
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include "chat_message.h"

class chat_session;

using boost::asio::ip::tcp;
using chat_message_queue = std::deque<chat_message>;
using chat_session_ptr = std::shared_ptr<chat_session>;

class chat_room {
 public:
  void join(chat_session_ptr participant);
  void leave(chat_session_ptr participant);
  void deliver(const chat_message& msg);

 private:
  std::set<chat_session_ptr> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

class chat_session : public std::enable_shared_from_this<chat_session> {
 public:
  chat_session(tcp::socket socket, chat_room& room)
      : socket_(std::move(socket)), room_(room) {}

  void start() {
    std::cout << "---- Start ----" << std::endl;
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg) {
    // first false
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    // just first come in this
    //! 防止重复调用
    if (!write_in_progress) {
      std::cout << "---- write ----" << std::endl;
      do_write();
    }
  }

 private:
  void do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](const boost::system::error_code& e, std::size_t length) {
          if (!e && read_msg_.decode_header()) {
            std::cout << "---- do_read_header ----" << std::endl;
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
        [this, self](const boost::system::error_code& error,
                     std::size_t length) {
          if (!error) {
            std::cout << "---- do_read_body ----" << std::endl;
            // room_.deliver(read_msg_);
            handleMessage();
            do_read_header();
          } else
            room_.leave(shared_from_this());
        });
  }
  void handleMessage() {
    if (read_msg_.type() == 1) {
      const BindName* bind =
          reinterpret_cast<const BindName*>(read_msg_.body());
      m_name.assign(bind->name, bind->name + bind->nameLen);
    } else if (read_msg_.type() == 2) {
      const ChatInfomation* chat =
          reinterpret_cast<const ChatInfomation*>(read_msg_.body());
      m_chatInfomation.assign(chat->information,
                              chat->information + chat->inforLen);

      auto rinfo = buildRoomInfo();
      chat_message msg;
      msg.setMessage(3, &rinfo, sizeof(rinfo));
      room_.deliver(msg);
    } else {
    }
  }
  void do_write() {
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front().data(),
                            write_msgs_.front().length()),
        [this, self](const boost::system::error_code& error,
                     std::size_t length) {
          std::cout << "---- do_write ----" << error << std::endl;
          if (!error) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
              std::cout << "---- write mgs empty ----" << error << std::endl;
              do_write();
            }

          } else
            room_.leave(shared_from_this());
        });
  }
  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  std::string m_name;
  std::string m_chatInfomation;
  RoomInfomation buildRoomInfo() const {
    RoomInfomation info;
    info.name.nameLen = m_name.size();
    std::memcpy(info.name.name, m_name.data(), m_name.size());
    std::memcpy(info.chat.information, m_chatInfomation.data(),
                m_chatInfomation.size());
    return info;
  }
};

void chat_room::join(chat_session_ptr participant) {
  participants_.insert(participant);
  for (const auto& msg : recent_msgs_) participant->deliver(msg);
}

void chat_room::leave(chat_session_ptr participant) {
  participants_.erase(participant);
}

void chat_room::deliver(const chat_message& msg) {
  recent_msgs_.push_back(msg);
  while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();
  std::cout << "---- deliver of all ----" << std::endl;
  for (auto& participant : participants_) participant->deliver(msg);
}

class chat_server {
 public:
  chat_server(boost::asio::io_service& io_service,
              const tcp::endpoint& endpoint)
      : acceptor_(io_service, endpoint), socket_(io_service) {
    do_accept();
  }

 private:
  void do_accept() {
    acceptor_.async_accept(
        socket_, [this](const boost::system::error_code& error) {
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

  return 0;
}
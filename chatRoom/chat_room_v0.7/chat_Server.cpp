/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 21:40:36
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 21:57:58
 * @Description: chat server by json and multithread and log and guard for wrap
 */

//! 瓶颈在 deliver 那块太多IO操作，msg不传引用是因为 msg 是一个临时变量
/*
 * chat_room: 控制交流队列和房间的出入
 * chat_session: 读写数据
 * chat_server: 监听分发socket
 */
//! 回调是安全的，但是回调的函数是不安全的
#include <boost/asio.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <utility>

#include "JSONObj.h"
#include "chat_message.h"

class chat_session;

using boost::asio::ip::tcp;
using chat_message_queue = std::deque<chat_message>;
using chat_session_ptr = std::shared_ptr<chat_session>;
std::chrono::system_clock::time_point base;

class chat_room {
 public:
  void join(chat_session_ptr participant);
  void leave(chat_session_ptr participant);
  void deliver(const chat_message& msg);
  chat_room(boost::asio::io_service& io_service) : m_strand(io_service) {}

 private:
  // std::mutex m_mutex;
  std::set<chat_session_ptr> participants_;
  enum { max_recent_msgs = 100000000000 };
  chat_message_queue recent_msgs_;
  boost::asio::io_service::strand m_strand;
  std::mutex m_mutex;
};

class chat_session : public std::enable_shared_from_this<chat_session> {
 public:
  chat_session(tcp::socket socket, chat_room& room)
      : socket_(std::move(socket)),
        room_(room),
        m_strand(socket.get_io_service()) {}

  void start() {
    std::cout << "---- Start ----" << std::endl;
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg) {
    m_strand.post([this, msg] {
      bool write_in_progress = !write_msgs_.empty();
      write_msgs_.push_back(msg);
      //! 防止重复调用
      if (!write_in_progress) {
        do_write();
      }
    });
  }

 private:
  void do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        m_strand.wrap([this, self](const boost::system::error_code& e,
                                   std::size_t length) {
          if (!e && read_msg_.decode_header()) {
            do_read_body();
          } else {
            room_.leave(shared_from_this());
          }
        }));
  }

  void do_read_body() {
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        m_strand.wrap([this, self](const boost::system::error_code& error,
                                   std::size_t length) {
          if (!error) {
            // room_.deliver(read_msg_);
            handleMessage();
            do_read_header();
          } else
            room_.leave(shared_from_this());
        }));
  }
  ptree toPtree() {
    ptree obj;
    std::stringstream ss(std::string(
        read_msg_.body(), read_msg_.body() + read_msg_.body_length()));
    boost::property_tree::read_json(ss, obj);
    return obj;
  }

  void handleMessage() {
    auto n = std::chrono::system_clock::now() - base;
    std::cout
        << "handleMessage: " << std::this_thread::get_id() << "time = "
        << std::chrono::duration_cast<std::chrono::milliseconds>(n).count()
        << std::endl;
    if (read_msg_.type() == 1) {
      auto nameTree = toPtree();
      m_name = nameTree.get<std::string>("name");
      std::cout << "BindName" << m_name << std::endl;
    } else if (read_msg_.type() == 2) {
      auto chat = toPtree();
      m_chatInfomation = chat.get<std::string>("information");
      auto rinfo = buildRoomInfo();
      chat_message msg;
      msg.setMessage(3, rinfo);
      std::cout << "RoomChat" << m_chatInfomation << std::endl;
      std::cout << "deliver" << std::endl;
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
        m_strand.wrap([this, self](const boost::system::error_code& error,
                                   std::size_t length) {
          if (!error) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
              do_write();
            }

          } else
            room_.leave(shared_from_this());
        }));
  }

  std::string buildRoomInfo() const {
    ptree tree;
    tree.put("name", m_name);
    tree.put("information", m_chatInfomation);
    return ptreeToJsonString(tree);
  }

  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  std::string m_name;
  std::string m_chatInfomation;
  boost::asio::io_service::strand m_strand;
  // std::mutex m_mutex;
};

void chat_room::join(chat_session_ptr participant) {
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_strand.post([this, participant] {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_) participant->deliver(msg);
  });
}

void chat_room::leave(chat_session_ptr participant) {
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_strand.post([this, participant] { participants_.erase(participant); });
}

void chat_room::deliver(const chat_message& msg) {
  // std::lock_guard<std::mutex> lock(m_mutex);
  m_strand.post([this, msg]() {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();
    for (auto& participant : participants_) participant->deliver(msg);
  });
}

class chat_server {
 public:
  chat_server(boost::asio::io_service& io_service,
              const tcp::endpoint& endpoint)
      : acceptor_(io_service, endpoint),
        socket_(io_service),
        room_(io_service) {
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

void init() {
  boost::log::add_file_log("sample.log");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);
}

int main(int argc, char* argv[]) {
  try {
    init();
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message: ";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message: ";
    BOOST_LOG_TRIVIAL(info) << "A information severity message: ";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message: ";
    BOOST_LOG_TRIVIAL(error) << "A error severity message: ";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message: ";

    if (argc < 2) {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }
    base = std::chrono::system_clock::now();

    boost::asio::io_service io_service;
    std::list<chat_server> servers;
    for (int i = 0; i < argc; ++i) {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_service, endpoint);
    }
    //! 多线程
    std::cout << "MainThread"
              << "=" << std::this_thread::get_id() << std::endl;
    std::vector<std::thread> threadGroup;
    for (int i = 0; i < 50; ++i) {
      std::cout << "T" << i << "=" << std::this_thread::get_id() << std::endl;
      threadGroup.emplace_back([&io_service]() { io_service.run(); });
    }
    io_service.run();

    for (auto& t : threadGroup) {
      t.join();
    }

  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  BOOST_LOG_TRIVIAL(info) << "safe release all resource";
  return 0;
}
/* 模板成员写法
template <typename T>
T toObj() {
  T obj;
  std::stringstream ss(std::string(read_msg_.body(),
                                   read_msg_.body() + read_msg_.body_length()));
  boost::archive::text_iarchive oa(ss);
  oa&& obj;
  return obj;
}
*/
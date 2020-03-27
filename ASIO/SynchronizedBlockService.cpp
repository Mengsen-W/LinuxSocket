/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 10:46:12
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 13:22:29
 * @Description: Synchronized Block Services
 */

#include <boost/asio.hpp>
#include <ctime>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

static std::string make_daytime_string() {
  std::time_t now = std::time(nullptr);
  return std::ctime(&now);  // return const char*
}

int main() {
  try {
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 13));
    std::cout << "start Service" << std::endl;
    for (;;) {
      tcp::socket socket(io_service);
      acceptor.accept(socket);

      std::string message = make_daytime_string();

      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
      std::cout << "client bye " << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  std::cout << "bye bye" << std::endl;
  return 0;
}
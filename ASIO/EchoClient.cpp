/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 18:58:56
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 12:20:54
 * @Description: Echo Client
 */

#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: main <host> <port>\n";
      return 1;
    }
    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({argv[1], argv[2]}));

    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = std::strlen(request);
    boost::asio::write(s, boost::asio::buffer(request, request_length));

    char replay[max_length];
    size_t replay_length =
        boost::asio::read(s, boost::asio::buffer(replay, request_length));
    std::cout << "Replay is: ";
    std::cout.write(replay, replay_length);
    std::cout << std::endl;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
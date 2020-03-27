/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 09:50:22
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-27 10:34:16
 * @Description: Asynchronous Block Client to get time now
 */

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    boost::asio::io_service io_service;

    //* 注册解析器--解析问题
    tcp::resolver reslover(io_service);
    //* 注册问题--确定协议
    tcp::resolver::query query(argv[1], "daytime");
    //* 解析协议确定的地址和端口号
    tcp::resolver::iterator endpoint_iterator = reslover.resolve(query);

    //* 注册socket
    tcp::socket socket(io_service);
    //* 链接socket和解析出的协议
    boost::asio::connect(socket, endpoint_iterator);

    for (;;) {
      //* 接受数据
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break;         //* 服务器主动断开
      else if (error)  //* 出现错误
        throw boost::system::system_error(error);
      std::cout.write(buf.data(), len);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

//! query的参数
//* 第一个参数是输入的地址或网址 本地配置文件 /etc/hosts
//* 第二个参数可以输入端口号或者一个协议名称 /etc/servicesso "daytime" = 13/tcl

//! read_some
//! 是阻塞的若有buffer里面就会一直读,若没有数据没有返回错误就会一直请求
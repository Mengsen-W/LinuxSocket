/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 21:43:31
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 10:10:44
 * @Description: struct of chat message
 */

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <cstdio>
#include <cstdlib>
#include <string>

#include "Protocal.pb.h"
#include "structHeader.h"

class chat_message {
 public:
  enum { header_length = sizeof(Header) };
  enum { max_body_length = 512 };

  chat_message() {}

  const char* data() const { return data_; }

  char* data() { return data_; }

  std::size_t length() const { return header_length + m_header.bodySize; }

  const char* body() const { return data_ + header_length; }

  char* body() { return data_ + header_length; }

  int type() const { return m_header.type; }

  std::size_t body_length() const { return m_header.bodySize; }

  void setMessage(int messageType, const void* buffer, size_t bufferSize) {
    assert(bufferSize <= max_body_length);
    m_header.bodySize = bufferSize;
    m_header.type = messageType;
    std::memcpy(body(), buffer, bufferSize);
    std::memcpy(data(), &m_header, sizeof(m_header));
  }
  void setMessage(int messageType, const std::string& buffer) {
    setMessage(messageType, buffer.data(), buffer.size());
  }

  bool decode_header() {
    std::memcpy(&m_header, data(), header_length);
    if (m_header.bodySize > max_body_length) {
      std::cout << "---- decode_header false ----" << std::endl;
      std::cout << "Body size = " << m_header.bodySize << " of "
                << m_header.type << std::endl;
      return false;
    }

    std::cout << "---- decode_header success ----" << std::endl;
    std::cout << "Body size = " << m_header.bodySize << " type  of "
              << m_header.type << std::endl;
    return true;
  }

  // void encode_header() {
  //   char header[header_length + 1] = "";
  //   std::sprintf(header, "%4d", static_cast<int>(body_length_));
  //   std::memcpy(data_, header, header_length);
  //   std::cout << "---- encode_header success ----" << std::endl;
  // }

 private:
  char data_[header_length + max_body_length];
  Header m_header;
};

#endif  // chat_message
/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-27 21:43:31
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 15:53:11
 * @Description: struct of chat message
 */

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <cstdio>
#include <cstdlib>
#include <string>

class chat_message {
 public:
  enum { header_length = 4 };
  enum { max_body_length = 512 };

  chat_message() : body_length_(0) {}

  const char* data() const { return data_; }

  char* data() { return data_; }

  std::size_t length() const { return header_length + body_length_; }

  const char* body() const { return data_ + header_length; }

  char* body() { return data_ + header_length; }

  std::size_t body_length() const { return body_length_; }

  void body_length(std::size_t new_length) {
    body_length_ = new_length;
    if (body_length_ > max_body_length) body_length_ = max_body_length;
  }

  bool decode_header() {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    body_length_ = std::atoi(header);

    if (body_length_ > max_body_length) {
      body_length_ = 0;
      std::cout << "decode_header false ----" << std::endl;
      return false;
    }

    std::cout << "decode_header success" << std::endl;
    return true;
  }

  void encode_header() {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);
    std::cout << "encode_header success" << std::endl;
  }

 private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
};

#endif  // chat_message
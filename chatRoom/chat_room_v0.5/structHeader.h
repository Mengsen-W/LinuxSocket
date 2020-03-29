/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-28 16:59:26
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-29 11:48:49
 * @Description: struct header
 */

#ifndef CHAT_MESSAGE_STRUCT_H
#define CHAT_MESSAGE_STRUCT_H
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "JSONObj.h"
#include "Protocal.pb.h"
#include "chat_message.h"
#include "serialization.h"

struct Header {
  int bodySize;
  int type;
};

struct BindName {
  char name[32] = {NULL};
  int nameLen;
};

struct ChatInfomation {
  char information[256] = {NULL};
  int inforLen;
};

struct RoomInfomation {
  BindName name;
  ChatInfomation chat;
};

template <typename T>
std::string serialize(const T& obj) {
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  // 这里是类的模板参数 友元重载了 & 类模板指明了 类中哪个成员是操作者
  oa& obj;
  return ss.str();
}

bool parseMessage(const std::string& input, int* type,
                  std::string& outputbuffer) {
  auto pos = input.find_first_of(" ");

  if (pos == std::string::npos) return false;

  if (pos == 0) return false;

  std::string command = input.substr(0, pos);

  if (command == "BindName") {
    std::string name = input.substr(pos + 1);
    if (name.size() > 32) return false;
    if (type) *type = 1;
    PBindName bindName;
    bindName.set_name(name);
    auto ok = bindName.SerializeToString(&outputbuffer);
    return ok;
  } else if (command == "Chat") {
    std::cout << "chat message" << std::endl;
    std::string chat = input.substr(pos + 1);
    if (chat.size() > 256) return false;
    if (type) *type = 2;
    PChat pchat;
    pchat.set_information(chat);
    auto ok = pchat.SerializeToString(&outputbuffer);
    return ok;
  }
  return false;
}

#endif  // CHAT_MESSAGE_STRUCT_H
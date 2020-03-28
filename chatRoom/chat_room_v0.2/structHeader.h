/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-28 16:59:26
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-28 18:57:11
 * @Description: struct header
 */

#ifndef CHAT_MESSAGE_STRUCT_H
#define CHAT_MESSAGE_STRUCT_H
#include <cstdlib>
#include <cstring>
#include <iostream>

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
    BindName bindInfo;
    bindInfo.nameLen = name.size();
    memcpy(&(bindInfo.name), name.data(), name.size());
    auto buffer = reinterpret_cast<const char*>(&bindInfo);
    outputbuffer.assign(buffer, buffer + sizeof(bindInfo));
    return true;
  } else if (command == "Chat") {
    std::string chat = input.substr(pos + 1);
    if (chat.size() > 256) return false;
    ChatInfomation Info;
    Info.inforLen = chat.size();
    std::memcpy(&(Info.information), chat.data(), chat.size());
    auto buffer = reinterpret_cast<const char*>(&Info);
    outputbuffer.assign(buffer, buffer + sizeof(Info));
    std::cout << sizeof(Info) << std::endl;
    if (type) *type = 2;
    return true;
  }
  return false;
}

#endif  // CHAT_MESSAGE_STRUCT_H
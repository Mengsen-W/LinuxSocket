/*
 * @Author: Mengsen.Wang
 * @Date: 2022-5-28 21:58
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time:
 * @Description: back up redis API
 */

#ifndef __MENGSEN__REDIES_H__
#define __MENGSEN__REDIES_H__

#include <hiredis/hiredis.h>

#include <iostream>
#include <string>

class redis {
 public:
  redis() : _connect(nullptr), _reply(nullptr) {}

  ~redis() {
    _connect = nullptr;
    _reply = nullptr;
  }

  bool connect(std::string host, int port) {
    _connect = redisConnect(host.c_str(), port);

    if (_connect != nullptr && _connect->err) {
      // err == 0 when there no err
      std::cout << "connect error = " << _connect->errstr << std::endl;
      return 0;
    }
    return 1;
  }

  std::string get(std::string key) {
    _reply =
        static_cast<redisReply*>(redisCommand(_connect, "GET %s", key.c_str()));
    std::string str = _reply->str;
    freeReplyObject(_reply);
    return str;
  }

  void set(std::string key, std::string value) {
    redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());
  }

 private:
  redisContext* _connect;
  redisReply* _reply;
};

#endif  // __MENGSEN__REDIES_H__

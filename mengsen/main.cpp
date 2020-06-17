/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-01 09:20:08
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-01 12:36:31
 */

// for testing

#include <json/json.h>

#include <iostream>
#include <string>

#include "./coroutine/schedule.h"
#include "./coroutine/types.h"
#include "./log/log.h"

// struct args {
//   int n;
// };

// static void foo(schedule_ptr S, void *ud) {}

// static void test(schedule_ptr c) {
//   args arg1 = {1};
//   // args arg2 = {1000};
//   std::cout << c->coroutine_running() << std::endl;

//   size_t co1 = c->coroutine_create(foo, &arg1);
//   std::cout << c->coroutine_running() << co1 << std::endl;
//   return;
// }

int testJson() {
  std::string strJsonContent =
      "{\"list\" : [{ \"camp\" : \"alliance\",\"occupation\" : \"paladin\",\"role_id\" : 1}, \
        {\"camp\" : \"alliance\",\"occupation\" : \"Mage\",\"role_id\" : 2}],\"type\" : \"roles_msg\",\"valid\" : true}";

  LOG_DEBUG << strJsonContent;
  std::string strJ1 = "\" name \" : \" mengsen \"";
  LOG_DEBUG << strJ1;
  strJ1 =
      "\" name \" : \
   \" mengsen \"";
  LOG_DEBUG << strJ1;
  strJ1 =
      "\" name \" : "
      "\" mengsen \"";
  LOG_DEBUG << strJ1;

  std::string strType;
  int nRoleDd = 0;
  std::string strOccupation;
  std::string strCamp;

  Json::Reader reader;
  Json::Value root;

  if (reader.parse(strJsonContent, root)) {
    // 获取非数组内容
    strType = root["type"].asString();
    LOG_DEBUG << "type is: " << strType;

    // 获取数组内容
    if (root["list"].isArray()) {
      int nArraySize = root["list"].size();
      for (int i = 0; i < nArraySize; i++) {
        nRoleDd = root["list"][i]["role_id"].asInt();
        strOccupation = root["list"][i]["occupation"].asString();
        strCamp = root["list"][i]["camp"].asString();

        LOG_DEBUG << "role_id is: " << nRoleDd;
        LOG_DEBUG << "occupation is: " << strOccupation;
        LOG_DEBUG << "camp is: " << strCamp;
      }
    }
  }

  return 0;
}

int main() {
  mengsen_log::initialize(mengsen_log::NonGuaranteedLogger(10),
                          "/home/Mengsen/LinuxSocket/mengsen/logfile/", "log",
                          1);
  LOG_INFO << "Hello world";
  testJson();
  // schedule_ptr c = std::make_shared<schedule>();
  // test(c);
  // 调用智能指针的析构函数
  return 0;
}
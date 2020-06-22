/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-01 09:20:08
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-22 21:37:24
 */

// for testing

#include <json/json.h>
#include <yaml-cpp/yaml.h>

#include <cassert>
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

void testYaml() {
  YAML::Node node;
  assert(node.IsNull());  //初始化的节点是Null类型
  node["key"] = "value";  //当你给它赋值键值对，它转变为Map类型
  // node.force_insert("key",
  // "value");//这个操作和上面等价，但是它不会检查是否存在"key"键，不推荐使用
  if (node["mascot"]) std::cout << node["mascot"].as<std::string>() << "\n";
  //单纯的查询操作不会增加一个key，当然上面的if不会执行

  node["number"] = 255;
  assert(node.IsMap());  // node是一个Map
  node["seq"].push_back("first element");
  node["seq"].push_back(
      "second element");  // node的seq下是Sequence类型，有两个参数

  YAML::Node node_2;
  node_2.push_back("first item");
  //如果你不给node_2键值对，它是一个sequence类型
  node_2.push_back("second_item");
  node_2.push_back("third_item");
  std::vector<int> v = {1, 3, 5, 7, 9};  //给node_2插入了一个Sequence
  node_2.push_back(v);
  assert(node_2.IsSequence());  //当然，node_2仍然是一个Sequence

  assert(node_2[0].as<std::string>() == "first item");
  //对于Sequence类型，你可以使用它的下标来访问
  //注意这里as<T>是一个模板转换，node_2[0]的type是NodeType::Scalar
  auto it = node_2.begin();
  for (; it != node_2.end(); it++) std::cout << *(it) << std::endl;
  //当然，你也可以用迭代器来访问
  //他们的类型分别是NodeType::Scalar，NodeType::Scalar，NodeType::Scalar，NodeType::Sequence
  //取值时记得使用as进行模板转换
  node_2["key"] = "value";
  assert(node_2.IsMap());  //一旦node_2接收到键值对，它转变为Map类型
  assert(node_2[0].as<std::string>() ==
         "first item");     //此时，Sequence时的下标变为它的key值
  node["node_2"] = node_2;  //将node_2作为node的一个子项
  node["pointer_to_first_element"] =
      node["seq"][0];  //你也可以给已有的node设置一个别名，类似于一个指针
  assert(node["pointer_to_first_element"].as<std::string>() ==
         "first element");  //你可以通过这个指针访问那个node

  node.remove(node["seq"][0]);  //你可以通过指定一个node来删除它
  node.remove("pointer_to_first_element");  //你也可以通过指定key来删除它
}

int main() {
  mengsen_log::initialize(mengsen_log::NonGuaranteedLogger(10),
                          "/home/Mengsen/LinuxSocket/mengsen/logfile/", "log",
                          1);
  LOG_INFO << "Hello world";
  testJson();
  testYaml();
  // schedule_ptr c = std::make_shared<schedule>();
  // test(c);
  // 调用智能指针的析构函数
  return 0;
}
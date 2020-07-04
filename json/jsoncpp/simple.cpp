/*
 * @Author: Mengsen.Wang
 * @Date: 2020-07-04 21:42:06
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-07-04 21:44:02
 */

#include <json/json.h>

#include <iostream>
#include <string>

using namespace std;

int main() {
  string strJsonContent =
      "{\"list\" : [{ \"camp\" : \"alliance\",\"occupation\" : \"paladin\",\"role_id\" : 1}, \
        {\"camp\" : \"alliance\",\"occupation\" : \"Mage\",\"role_id\" : 2}],\"type\" : \"roles_msg\",\"valid\" : true}";
  string strJsontest = "[[1, 2, 3, 4], [1, 2, 3, 4], [1, 2, 3, 4]]";
  string strType;
  int nRoleDd = 0;
  string strOccupation;
  string strCamp;

  Json::Reader reader;
  Json::Value root;

  vector<vector<int>> output;

  if (reader.parse(strJsontest, root)) {
    int size_y = root.size();
    int size_x = root[0].size();

    for (int i = 0; i < size_y; i++) {
      vector<int> temp;
      for (int j = 0; j < size_x; j++) {
        temp.push_back(root[i][j].asInt());
      }
      output.push_back(temp);
    }
  }

  for (auto &vec : output) {
    for (int num : vec) {
      std::cout << num;
    }
    std::cout << std::endl;
  }

  // 用reader解析str，并存入root
  if (reader.parse(strJsonContent, root)) {
    // 获取非数组内容
    strType = root["type"].asString();
    cout << "type is: " << strType << endl;

    // 获取数组内容
    if (root["list"].isArray()) {
      int nArraySize = root["list"].size();
      for (int i = 0; i < nArraySize; i++) {
        nRoleDd = root["list"][i]["role_id"].asInt();
        strOccupation = root["list"][i]["occupation"].asString();
        strCamp = root["list"][i]["camp"].asString();

        cout << "role_id is: " << nRoleDd << endl;
        cout << "occupation is: " << strOccupation << endl;
        cout << "camp is: " << strCamp << endl;
      }
    }
  }
  return 0;
}
/*
 * @Author: Mengsen.Wang
 * @Date: 2020-07-03 21:28:50
 * @Last Modified by:   Mengsen.Wang
 * @Last Modified time: 2020-07-03 21:28:50
 */

#include <json/json.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct TreeNode {
  int val;
  TreeNode *left;
  TreeNode *right;
  TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

bool boolToString(bool input) { return input ? "True" : "False"; }

int stringToInteger(string input) { return stoi(input); }

void trimLeftTrailingSpaces(string &input) {
  input.erase(input.begin(), find_if(input.begin(), input.end(),
                                     [](int ch) { return !isspace(ch); }));
}

void trimRightTrailingSpaces(string &input) {
  input.erase(
      find_if(input.rbegin(), input.rend(), [](int ch) { return !isspace(ch); })
          .base(),
      input.end());
}

vector<int> stringToIntegerVector(string input) {
  vector<int> output;
  trimLeftTrailingSpaces(input);
  trimRightTrailingSpaces(input);
  input = input.substr(1, input.length() - 2);
  stringstream ss;
  ss.str(input);
  string item;
  char delim = ',';
  while (getline(ss, item, delim)) {
    output.push_back(stoi(item));
  }
  return output;
}

string integerVectorToString(vector<int> list, int length = -1) {
  if (length == -1) {
    length = list.size();
  }

  if (length == 0) {
    return "[]";
  }

  string result;
  for (int index = 0; index < length; index++) {
    int number = list[index];
    result += to_string(number) + ", ";
  }
  return "[" + result.substr(0, result.length() - 2) + "]";
}

void stringTo2DIntegerVector() {
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

  // // 用reader解析str，并存入root
  // if (reader.parse(strJsonContent, root)) {
  //   // 获取非数组内容
  //   strType = root["type"].asString();
  //   cout << "type is: " << strType << endl;

  //   // 获取数组内容
  //   if (root["list"].isArray()) {
  //     int nArraySize = root["list"].size();
  //     for (int i = 0; i < nArraySize; i++) {
  //       nRoleDd = root["list"][i]["role_id"].asInt();
  //       strOccupation = root["list"][i]["occupation"].asString();
  //       strCamp = root["list"][i]["camp"].asString();

  //       cout << "role_id is: " << nRoleDd << endl;
  //       cout << "occupation is: " << strOccupation << endl;
  //       cout << "camp is: " << strCamp << endl;
  //     }
  //   }
  // }
  return;
}

string treeNodeToString(TreeNode *root) {
  if (root == nullptr) {
    return "[]";
  }

  string output = "";
  queue<TreeNode *> q;
  q.push(root);
  while (!q.empty()) {
    TreeNode *node = q.front();
    q.pop();

    if (node == nullptr) {
      output += "null, ";
      continue;
    }

    output += to_string(node->val) + ", ";
    q.push(node->left);
    q.push(node->right);
  }
  return "[" + output.substr(0, output.length() - 2) + "]";
}

int main() {
  string line;
  while (getline(cin, line)) {
    int s = stringToInteger(line);
    std::cout << "Integer = " << s << std::endl;
    getline(cin, line);
    vector<int> nums = stringToIntegerVector(line);
    for (int num : nums) {
      std::cout << num << std::endl;
    }

    // int ret = Solution().minSubArrayLen(s, nums);

    // string out = to_string(ret);
    // cout << out << endl;
  }
  return 0;
}

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

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
  std::string text = "[1,2],[1,2]";
  std::regex ws_re("/[./],");  // whitespace
  std::vector<std::string> v(
      std::sregex_token_iterator(text.begin(), text.end(), ws_re, -1),
      std::sregex_token_iterator());
  for (auto &&s : v) std::cout << s << "\n";
}

int main() {
  stringTo2DIntegerVector();
  string line;
  while (getline(cin, line)) {
    // int s = stringToInteger(line);
    // std::cout << "Integer = " << s << std::endl;
    // getline(cin, line);
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

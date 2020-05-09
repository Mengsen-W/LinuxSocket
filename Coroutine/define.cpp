/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-08 21:45:39
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-08 21:46:02
 * @Description: 用 define 的方式实现协程
 */

#include <iostream>

using namespace std;

#define co_yield(ret) \
  ++_ip;              \
  return ret;

int test() {
  static int _ip = 0;

  switch (_ip) {
    case 0:
      static int count;
      count = 0;
      cout << count << endl;
      co_yield(_ip);
    case 1:
      count++;
      cout << count << endl;
      co_yield(_ip);
    case 2:
      count++;
      cout << count << endl;
  }

  _ip = 0;
  return _ip;
}

int main(void) {
  test();
  cout << 'A' << endl;
  test();
  cout << 'B' << endl;
  test();
  cout << 'C' << endl;
  return 0;
}

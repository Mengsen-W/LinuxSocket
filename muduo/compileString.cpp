/*
 * @Author: Mengsen.Wang
 * @Date: 2020-05-15 10:54:03
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-05-15 19:32:36
 */

#include <cassert>
#include <cstring>
#include <iostream>

class SourceFile {
 public:
  template <int N>
  constexpr SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
    // maybe no '/0' in the end
    // find last position of '/'
    const char* slash = strrchr(data_, '/');  // builtin function
    std::cout << "const char (&)arr[N]" << std::endl;
    if (slash) {
      data_ = slash + 1;
      size_ -= static_cast<int>(data_ - arr);
    }
  }

  explicit SourceFile(const char* filename) : data_(filename) {
    const char* slash = strrchr(filename, '/');
    std::cout << "const char* filename" << std::endl;
    if (slash) {
      data_ = slash + 1;
    }
    size_ = static_cast<int>(strlen(data_));
  }

  const char* data_;  // before '/'
  int size_;          // no '/0'
};

int main() {
  // static_assert(sizeof(void*) == 4, "false 64 bit plat");
  printf("%s, %ld\n", __FILE__, sizeof(__FILE__));
  SourceFile s(__FILE__);
  // std::cout << s.size_ << std::endl;
  // std::cout << s.data_ << std::endl;
  return 0;
}
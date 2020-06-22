/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-22 21:42:00
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-22 21:42:34
 */

template <class T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-22 21:42:00
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-25 18:24:37
 */

#ifndef __MENGSEN_UTILITY_H__
#define __MENGSEN_UTILITY_H__

#include <cxxabi.h>

template <class T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

class FSUtil {
 public:
  static void ListAllFile(std::vector<std::string>& files,
                          const std::string& path, const std::string& subfix);
  static bool Mkdir(const std::string& dirname);
  static bool IsRunningPidfile(const std::string& pidfile);
  static bool Rm(const std::string& path);
  static bool Mv(const std::string& from, const std::string& to);
  static bool Realpath(const std::string& path, std::string& rpath);
  static bool Symlink(const std::string& frm, const std::string& to);
  static bool Unlink(const std::string& filename, bool exist = false);
  static std::string Dirname(const std::string& filename);
  static std::string Basename(const std::string& filename);
  static bool OpenForRead(std::ifstream& ifs, const std::string& filename,
                          std::ios_base::openmode mode);
  static bool OpenForWrite(std::ofstream& ofs, const std::string& filename,
                           std::ios_base::openmode mode);
};

#endif  // __MENGSEN_UTILITY_H__

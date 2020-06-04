/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-04 21:38:59
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-04 22:14:45
 */

#include <cstdint>
#include <memory>  // for unique_ptr
#include <string>  // for size_t

#ifndef __MENGSEN_LOG_H__
#define __MENGSEN_LOG_H__
namespace mengsen_log {

/**
 * enum class of log level
 */
enum class LogLevel : uint8_t {
  DEBUG,
  INFO,
  NOTICE,
  WARNING,
  ERROR,
  CRIT,
  ALERT,
  EMERGE,
  FATAL
};

class LogLine {
 public:
  LogLine(LogLevel level, const char *filename, const char *function,
          uint32_t line);
  ~LogLine();

 private:
  size_t _bytes_used;
  size_t _buffer_size;
  std::unique_ptr<char[]> _heap_buffer;
  char _stack_buffer[  // reserved
      256 - 2 * sizeof(size_t) - sizeof(decltype(_buffer_size)) - 8];
};

}  // namespace mengsen_log
#endif  // __MENGSEN_LOG_H__

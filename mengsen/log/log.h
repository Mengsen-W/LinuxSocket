/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-04 21:38:59
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-25 19:09:14
 */

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <iostream>  // for ostream
#include <memory>    // for unique_ptr
#include <string>    // for size_t

#ifndef __MENGSEN_LOG_H__
#define __MENGSEN_LOG_H__

namespace mengsen_log {

/**
 * @brief: enum class of log level
 * @enum:
 * DEBUG,INFO,NOTICE,WARNING,
 * ERROR,CRIT,ALERT,EMERGE,FATAL
 */
enum class LogLevel : uint8_t {
  DEBUG,
  INFO,
  NOTICE,
  WARN,
  ERROR,
  CRIT,
  ALERT,
  EMERGE,
  FATAL
};

/**
 * @brief: to set log line
 */
class LogLine {
 public:
  LogLine(LogLevel level, const char *filename, const char *function,
          uint32_t line);
  ~LogLine();
  LogLine(LogLine &&) = default;
  LogLine &operator=(LogLine &&) = default;

  void stringify(std::ostream &os);

  LogLine &operator<<(char arg);
  LogLine &operator<<(const std::string &arg);
  LogLine &operator<<(uint32_t arg);
  LogLine &operator<<(int32_t arg);
  LogLine &operator<<(uint64_t arg);
  LogLine &operator<<(int64_t arg);
  LogLine &operator<<(double arg);
  LogLine &operator<<(const YAML::Node &node);

  template <size_t N>
  LogLine &operator<<(const char (&arg)[]) {
    encode(string_literal_t(arg));
    return *this;
  }

  /**
   * @brief: const char* operator << version
   */
  template <typename Arg>
  typename std::enable_if<std::is_same<Arg, const char *>::value,
                          LogLine &>::type
  operator<<(const Arg &arg) {
    encode(arg);
    return *this;
  }

  /**
   * @brief: char* operator << version
   */
  template <typename Arg>
  typename std::enable_if<std::is_same<Arg, char *>::value, LogLine &>::type
  operator<<(const Arg &arg) {
    encode(arg);
    return *this;
  }

  /**
   * @brief: no '\0'
   */
  struct string_literal_t {
    explicit string_literal_t(const char *s) : _s(s) {}
    const char *_s;
  };

 private:
  char *buffer();

  template <typename Arg>
  void encode(Arg arg);
  template <typename Arg>
  void encode(Arg arg, uint8_t type_id);
  void encode(char *arg);
  void encode(const char *arg);
  void encode(string_literal_t arg);
  void encode_c_string(const char *arg, size_t length);

  void resize_buffer_if_needed(size_t additional_bytes);

  void stringify(std::ostream &os, char *start, const char *const end);
  size_t _bytes_used;
  size_t _buffer_size;
  std::unique_ptr<char[]> _heap_buffer;
  char _stack_buffer[  // reserved
      256 - 2 * sizeof(size_t) - sizeof(decltype(_buffer_size)) - 8];
};

/**
 * @brief: set logger pointer and add logline
 */
struct Log {
  bool operator==(LogLine &);
};

/**
 * @brief: set log level
 * @param: LogLevel level
 * @return: void
 */
void set_log_level(LogLevel level);

/**
 * @brief: Determines whether the log level is higher than the set log level
 * @param: LogLevel level
 * @return: if higher than set return true else return false
 */
bool is_logged(LogLevel level);

/**
 * Non guaranteed Log
 * Using a ring buffer to hold log lines
 * when the ring buffer is full, the previous log will be dropped
 * doesn't block producer even if the ring buffer is full
 * LogLines are 256 bytes
 * ring buffer size = ring_buffer_size_mb* 1024* 1024/ 256
 */
struct NonGuaranteedLogger {
  NonGuaranteedLogger(uint32_t ring_buffer_size_mb)
      : _ring_buffer_size_mb(ring_buffer_size_mb) {}

  uint32_t _ring_buffer_size_mb;
};

/**
 * guaranteed log
 */
struct GuaranteedLogger {};

/**
 * @brief:ensure initialize() is called proior to any log
 * @param:
 *  NonGuranteedLogger gl, [used to set ring buffer]
 *  const std::string &log_directorary, [where to creat logs]
 *  const std::string &log_file_name, [log file name]
 *  uint32_t log_file_roll_size_mb [mega bytes after roll next log file]
 * @return: void
 */
void initialize(NonGuaranteedLogger ngl, const std::string &log_directorary,
                const std::string &log_file_name,
                uint32_t log_file_roll_size_mb);

/**
 * @brief: Guaranteed version of initialize
 */
void initialize(GuaranteedLogger gl, const std::string &log_directorary,
                const std::string &log_file_name,
                uint32_t log_file_roll_size_mb);

}  // namespace mengsen_log

/**
 * @brief: warp init Log class and LogLine class
 * @param: LEVEL [LogLevel]
 * @return: bool
 */
#define LOG(LEVEL)      \
  mengsen_log::Log() == \
      mengsen_log::LogLine(LEVEL, __FILE__, __func__, __LINE__)

// log debug
#define LOG_DEBUG                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::DEBUG) && \
      LOG(mengsen_log::LogLevel::DEBUG)
// log info
#define LOG_INFO                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::INFO) && \
      LOG(mengsen_log::LogLevel::INFO)
// log notices
#define LOG_NOTICE                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::NOTICE) && \
      LOG(mengsen_log::LogLevel::NOTICE)
// log warn
#define LOG_WARN                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::WARN) && \
      LOG(mengsen_log::LogLevel::WARN)
// log error
#define LOG_ERROR                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::ERROR) && \
      LOG(mengsen_log::LogLevel::ERROR)
// log crit
#define LOG_CRIT                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::CRIT) && \
      LOG(mengsen_log::LogLevel::CRIT)
// log alert
#define LOG_ALERT                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::ALERT) && \
      LOG(mengsen_log::LogLevel::ALERT)
// log emerge
#define LOG_EMERGE                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::EMERGE) && \
      LOG(mengsen_log::LogLevel::EMERGE)
// log fatal
#define LOG_FATAL                                         \
  mengsen_log::is_logged(mengsen_log::LogLevel::FATAL) && \
      LOG(mengsen_log::LogLevel::FATAL)

#endif  // __MENGSEN_LOG_H__

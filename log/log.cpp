/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-05 21:07:13
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-06 19:27:05
 */

#include "log.h"

#include <chrono>  // for time stamp
#include <thread>  // for thread

namespace {
// internal linkage mean internal visible

/**
 * @brief: get time stamp
 * @param: void
 * @return: microseconds since epoch
 */
uint64_t timestamp_now() {
  // cast time to microseconds
  return std::chrono::duration_cast<std::chrono::microseconds>(
             // get now time interger from since epoch
             std::chrono::high_resolution_clock::now().time_since_epoch())
      // convert to int64_t type
      .count();
}

/**
 * @brief: format time stamp like [YYYY-MM-DD hh:mm:ss.ssss]
 * @param:
 * std::ostream& os,[save formate log time]
 * uint64_t timestamp [time stamp]
 * @return: void
 */
void format_timestamp(std::ostream& os, uint64_t timestamp) {
  // all convert for time_t and CMT time
  // convert duration type form int64_t
  std::chrono::microseconds duration{timestamp};
  // convert duration time to time_point
  std::chrono::high_resolution_clock::time_point time_point{duration};
  // convert time_point to time_t
  std::time_t time_now =
      std::chrono::high_resolution_clock::to_time_t(time_point);
  // get GMT time from time_t
  tm* gmtime = std::gmtime(&time_now);

  // save format style time
  char buffer[32];
  // converts the time format to the format we want
  strftime(buffer, 32, "%Y-%m-%d %T.", gmtime);
  char microseconds[7];
  sprintf(microseconds, "%06llu", timestamp % 1000000);
  os << '[' << buffer << microseconds << ']';
}

/**
 * @brief: get thread id and saved id for thread static
 * @param: void
 * @return: std::thread::id
 */
std::thread::id this_thread_id() {
  static const thread_local std::thread::id id = std::this_thread::get_id();
  return id;
}

/**
 *  Gets the index location of the data type specified
 * in the tuple parameterlistn
 */
template <typename T, typename Tuple>
struct TupleIndex;

template <typename T, typename... Types>
struct TupleIndex<T, std::tuple<T, Types...>> {
  static constexpr const std::size_t value = 0;
};
template <typename T, typename U, typename... Types>
struct TupleIndex<T, std::tuple<U, Types...>> {
  static constexpr const std::size_t value =
      1 + TupleIndex<T, std::tuple<Types>>::value;
};

}  // anonymous namespace

namespace mengsen_log {
typedef std::tuple<char, uint32_t, uint64_t, int32_t, int64_t, double,
                   LogLine::string_literal_t, char*>
    SupportedTypes;

/**
 * @brief: convert LogLevel to string
 * @param: [in] LogLevel
 * @return: const char *
 */
const char* to_string(LogLevel loglevel) {
  switch (loglevel) {
#define LEVEL(name)    \
  case LogLevel::name: \
    return #name;

    LEVEL(DEBUG);
    LEVEL(INFO);
    LEVEL(NOTICE);
    LEVEL(WARNING)
    LEVEL(ERROR);
    LEVEL(CRIT);
    LEVEL(ALERT);
    LEVEL(EMERGE);
    LEVEL(FATAL);

#undef LEVEL
    default:
      return "unknow";
  }
}

/**
 * @brief: constructor for LogLine
 * all of parameter automatic input of macro definition
 * @param: [in] LogLevel loglevel
 * @param: [in] const char *file
 * @param: [in] const char *function
 * @param: [in] uint32_t line
 */
LogLine::LogLine(LogLevel loglevel, const char* file, const char* function,
                 uint32_t line)
    : _bytes_used(0), _buffer_size(sizeof(_stack_buffer)) {
  encode<uint64_t>(timestamp_now());
  encode<std::thread::id>(this_thread_id());
  encode<string_literal_t>(string_literal_t(file));
  encode<string_literal_t>(string_literal_t(function));
  encode<uint32_t>(line);
  encode<LogLevel>(loglevel);
}

/**
 * @brief: default destructor for LogLine
 */
LogLine::~LogLine() = default;

/**
 * @brief: encode arg to buffer
 * @param:[in] typename Arg arg
 * @return: void
 */
template <typename Arg>
void LogLine::encode(Arg arg) {
  *(reinterpret_cast<Arg*>(buffer())) = arg;
  _bytes_used += sizeof(arg);
}

/**
 * @brief: call resize_buffer_if_needed() and call single variable encode
 * @param:[in] Arg arg
 * @param:[in] uint8_t type_id
 * @return: void
 */
template <typename Arg>
void LogLine::encode(Arg arg, uint8_t type_id) {
  resize_buffer_if_needed(sizeof(Arg) + sizeof(uint8_t));
  encode<uint8_t>(typeid);
  encode<Arg>(arg);
}

/**
 * @brief:
 */
void LogLine::stringify(std::ostream& os) {
  // get space pointer
  char* b = !_heap_buffer ? _stack_buffer : _heap_buffer.get();
  // temp variable for stringify() double parameter version
  // or shuold be able to pass const reference
  const char* const end = b + _bytes_used;
  // get time stamp
  uint64_t timestamp = *(reinterpret_cast<uint64_t*>(b));
  // pass pointer time stamp
  b += sizeof(uint64_t);
  // get thread id
  std::thread::id threadid = *(reinterpret_cast<std::thread::id*>(b));
  // pass pointer thread id
  b += sizeof(std::thread::id);
  // get file name
  string_literal_t file = *(reinterpret_cast<string_literal_t*>(b));
  // pass pointer file name
  b += sizeof(string_literal_t);
  // get function name
  string_literal_t function = *(reinterpret_cast<string_literal_t*>(b));
  // pass pointer function name
  b += sizeof(string_literal_t);
  // get line number
  uint32_t line = *reinterpret_cast<uint32_t*>(b);
  // pass pointer line number
  b += sizeof(uint32_t);
  // get log level
  LogLevel loglevel = *reinterpret_cast<LogLevel*>(b);
  // pass pointer log level
  b += sizeof(LogLevel);

  // format timestamp
  format_timestamp(os, timestamp);

  // timestamp[loglevel][threadid][filename:functionname:line]
  os << '[' << to_string(loglevel) << ']' << '[' << threadid << ']' << '['
     << file._s << ':' << function._s << ':' << line << "] ";

  stringify(os, b, end);

  if (loglevel >= LogLevel::CRIT) {
    // Emergency log immediately output
    os.flush();
  }
  return;
}

}  // namespace mengsen_log

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-05 21:07:13
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-06 18:51:30
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

}  // namespace mengsen_log
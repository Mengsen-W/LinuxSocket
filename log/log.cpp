/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-05 21:07:13
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-06 19:27:05
 */

#include "log.h"

#include <atomic>   //for atomic and atomic_flag
#include <chrono>   // for time stamp
#include <cstring>  // for memcpy() strlen()
#include <fstream>  // for FileWriter class
#include <queue>    // QueueBuffer class
#include <thread>   // for thread

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
  _bytes_used += sizeof(Arg);
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

void LogLine::encode(const char* arg) {
  if (arg != nullptr) encode_c_string(arg, strlen(arg));
  return;
}

void LogLine::encode(char* arg) {
  if (arg != nullptr) encode_c_string(arg, strlen(arg));
  return;
}

void LogLine::encode_c_string(const char* arg, size_t length) {
  if (length == 0) return;

  resize_buffer_if_needed(1 + length + 1);
  char* b = buffer();
  auto type_id = TupleIndex<char*, SupportedTypes>::value;
  *(reinterpret_cast<uint8_t*>(b++)) = static_cast<uint8_t>(type_id);
  memcpy(b, arg, length + 1);
  _bytes_used += 1 + length + 1;
  return;
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

//? unknow
template <typename Arg>
char* decode(std::ostream& os, char* b, Arg* dummy) {
  Arg arg = *(reinterpret_cast<Arg*>(b));
  os << arg;
  return b + sizeof(Arg);
}
template <>
char* decode(std::ostream& os, char* b, LogLine::string_literal_t* dummy) {
  LogLine::string_literal_t s =
      *(reinterpret_cast<LogLine::string_literal_t*>(b));
  os << s._s;
  return b + sizeof(LogLine::string_literal_t);
}
template <>
char* decode(std::ostream& os, char* b, char** dummy) {
  while (*b != '\0') {
    os << *b;
    ++b;
  }
  return ++b;
}

void LogLine::stringify(std::ostream& os, char* start, const char* const end) {
  if (start == end) return;
  int type_id = static_cast<int>(*start);
  ++start;

  switch (type_id) {
#define CASE(num)                                                           \
  case num:                                                                 \
    stringify(                                                              \
        os,                                                                 \
        decode(os, start,                                                   \
               static_cast<std::tuple_element<num, SupportedTypes>::type*>( \
                   nullptr)),                                               \
        end);
    return;

    CASE(0);
    CASE(1);
    CASE(2);
    CASE(3);
    CASE(4);
    CASE(5);
    CASE(6);

#undef CASE
  }
}

char* LogLine::buffer() {
  return !_heap_buffer ? &_stack_buffer[_bytes_used]
                       : &(_heap_buffer.get())[_bytes_used];
}

/**
 * @brief: resize buffer if needed
 * @param: [in] size_t additional_bytes
 * @return: void
 */
void LogLine::resize_buffer_if_needed(size_t additional_bytes) {
  const size_t required_size = _bytes_used + additional_bytes;
  if (required_size <= _buffer_size) return;  // no need resize buffer size

  // need to resize buffer
  if (!_heap_buffer) {  // no heap space
    _buffer_size = std::max(static_cast<size_t>(512), required_size);
    _heap_buffer.reset(new char[_buffer_size]);
    memcpy(_heap_buffer.get(), _stack_buffer, _bytes_used);
    return;
  } else {  // has heap space but no need
    // copy and swap
    _buffer_size = std::max(static_cast<size_t>(512), required_size);
    std::unique_ptr<char[]> new_heap_buffer(new char[_buffer_size]);
    memcpy(new_heap_buffer.get(), _heap_buffer.get(), _bytes_used);
    _heap_buffer.swap(new_heap_buffer);
  }
  return;
}

LogLine& LogLine::operator<<(const std::string& arg) {
  encode_c_string(arg.c_str(), arg.size());
  return *this;
}

LogLine& LogLine::operator<<(int32_t arg) {
  encode<int32_t>(arg, TupleIndex<int32_t, SupportedTypes>::value);
  return *this;
}

LogLine& LogLine::operator<<(uint32_t arg) {
  encode<uint32_t>(arg, TupleIndex<uint32_t, SupportedTypes>::value);
  return *this;
}

LogLine& LogLine::operator<<(int64_t arg) {
  encode<int64_t>(arg, TupleIndex<int64_t, SupportedTypes>::value);
  return *this;
}

LogLine& LogLine::operator<<(uint64_t arg) {
  encode<uint64_t>(arg, TupleIndex<uint64_t, SupportedTypes>::value);
  return *this;
}

LogLine& LogLine::operator<<(double arg) {
  encode<double>(arg, TupleIndex<double, SupportedTypes>::value);
  return *this;
}

LogLine& LogLine::operator<<(char arg) {
  encode<char>(arg, TupleIndex<char, SupportedTypes>::value);
  return *this;
}

struct BufferBase {
  virtual ~BufferBase() = default;
  virtual void push(LogLine&& logline) = 0;
  virtual bool try_pop(LogLine& logline) = 0;
};

/**
 * @brief: set flag for atomic flag
 */
struct SpinLock {
  SpinLock(std::atomic_flag& flag) : _flag(flag) {
    while (_flag.test_and_set(std::memory_order_acquire))
      ;
  }
  ~SpinLock() { _flag.clear(std::memory_order_release); }

 private:
  std::atomic_flag& _flag;
};

/**
 * @brief Multi Producer Single Consumer Ring Buffer
 */
class RingBuffer : public BufferBase {
 public:
  struct alignas(64) Item {
    Item()
        : flag(ATOMIC_FLAG_INIT),
          written(0),
          logline(LogLevel::INFO, nullptr, nullptr, 0) {}
    std::atomic_flag flag;
    char written;
    char padding[256 - sizeof(std::atomic_flag) - sizeof(char) -
                 sizeof(LogLine)];
    LogLine logline;
  };

  RingBuffer(const size_t size)
      : _size(size),
        _ring(static_cast<Item*>(std::malloc(size * sizeof(Item)))),
        _write_index(0),
        _read_index(0) {
    for (size_t i = 0; i < _size; ++i) {
      new (&_ring[i]) Item();
    }
    static_assert(sizeof(Item) == 256, "Unexpected size != 256");
  }

  ~RingBuffer() {
    for (size_t i = 0; i < _size; ++i) {
      _ring[i].~Item();
    }
    std::free(_ring);
  }

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  void push(LogLine&& logline) override {
    unsigned int write_index =
        _write_index.fetch_add(1, std::memory_order_relaxed) % _size;
    Item& item = _ring[write_index];
    SpinLock spinlock(item.flag);
    item.logline = std::move(logline);
    item.written = 1;
  }

  bool try_pop(LogLine& logline) override {
    Item& item = _ring[_write_index % _size];
    SpinLock spinlock(item.flag);
    if (item.written == 1) {
      logline = std::move(item.logline);
      item.written = 0;
      ++_read_index;
      return true;
    }
    return false;
  }

 private:
  const size_t _size;
  Item* _ring;
  std::atomic<unsigned int> _write_index;
  char pad[64];
  unsigned int _read_index;
};

class Buffer {
 public:
  struct Item {
    Item(LogLine&& logline) : _logline(std::move(logline)) {}
    char padding[256 - sizeof(LogLine)];
    LogLine _logline;
  };

  // 8MB. Helps reduce mempry fragmentation
  static constexpr const size_t size = 32768;

  Buffer() : _buffer(static_cast<Item*>(std::malloc(size * sizeof(Item)))) {
    for (size_t i = 0; i <= size; ++i) {
      _write_state[i].store(0, std::memory_order_relaxed);
    }
    static_assert(sizeof(Item) == 256, "Unexpected size != 256");
  }
  ~Buffer() {
    unsigned int write_count = _write_state[size].load();
    for (size_t i = 0; i < write_count; ++i) {
      _buffer[i].~Item();
    }
    std::free(_buffer);
  }
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  /**
   * @brief: return true if we need to switch to next buffer
   * @param: [in] LogLine &&logline
   * @param: [in] const unsigned int write_index
   * @return: bool
   */
  bool push(LogLine&& logline, const unsigned int write_index) {
    new (&_buffer[write_index]) Item(std::move(logline));
    _write_state[write_index].store(1, std::memory_order_release);
    return _write_state[size].fetch_add(1, std::memory_order_acquire) + 1 ==
           size;
  }

  bool try_pop(LogLine& logline, const unsigned int read_index) {
    if (_write_state[read_index].load(std::memory_order_acquire)) {
      Item& item = _buffer[read_index];
      logline = std::move(item._logline);
      return true;
    }
    return false;
  }

 private:
  Item* _buffer;
  std::atomic<unsigned int> _write_state[size + 1];
};

// TODO
class QueueBuffer : public BufferBase {
 public:
  QueueBuffer(const QueueBuffer&) = delete;
  QueueBuffer& operator=(const QueueBuffer&) = delete;
  QueueBuffer()
      : _current_write_buffer(nullptr),
        _write_index(0),
        _flags(ATOMIC_FLAG_INIT),
        _read_index(0) {
    setup_next_write_buffer();
  }

  void push(LogLine&& logline) override {
    unsigned int write_index =
        _write_index.fetch_add(1, std::memory_order_relaxed);
    if (write_index < Buffer::size) {
      if (_current_write_buffer.load(std::memory_order_acquire)
              ->push(std::move(logline), write_index)) {
        setup_next_write_buffer();
      }
    } else {
      while (_write_index.load(std::memory_order_acquire) >= Buffer::size)
        ;
      push(std::move(logline));
    }
  }

  bool try_pop(LogLine& logline) override {
    if (_current_write_buffer == nullptr)
      _current_read_buffer = get_next_read_buffer();
    Buffer* read_buffer = _current_read_buffer;
    if (read_buffer == nullptr) return false;
    if (bool success = read_buffer->try_pop(logline, _read_index)) {
      ++_read_index;
      if (_read_index == Buffer::size) {
        _read_index = 0;
        _current_read_buffer = nullptr;
        SpinLock spinlock(_flags);
        _buffers.pop();
      }
      return true;
    }
    return false;
  }

 private:
  void setup_next_write_buffer() {
    std::unique_ptr<Buffer> next_write_buffer(new Buffer);
    _current_write_buffer.store(next_write_buffer.get(),
                                std::memory_order_release);
    SpinLock spinlock(_flags);
    _buffers.push(std::move(next_write_buffer));
    _write_index.store(0, std::memory_order_relaxed);
  }

  Buffer* get_next_read_buffer() {
    SpinLock spinlock(_flags);
    return _buffers.empty() ? nullptr : _buffers.front().get();
  }

 private:
  std::queue<std::unique_ptr<Buffer>> _buffers;
  std::atomic<Buffer*> _current_write_buffer;
  Buffer* _current_read_buffer;
  std::atomic<unsigned int> _write_index;
  std::atomic_flag _flags;
  unsigned int _read_index;
};

class FileWriter {
 public:
  FileWriter(const std::string& log_directorary,
             const std::string& log_file_name, uint32_t log_file_roll_size_mb)
      : _log_file_roll_size_bytes(log_file_roll_size_mb + 1024 * 1024),
        _name(log_directorary + log_file_name) {
    roll_file();
  }

  void write(LogLine& logline) {
    std::streampos pos = _os->tellp();
    logline.stringify(*_os);
    _bytes_written += _os->tellp() - pos;
    if (_bytes_written > _log_file_roll_size_bytes) roll_file();
    return;
  }

 private:
  void roll_file() {
    if (_os) {
      _os->flush();
      _os->close();
    }
    _bytes_written = 0;
    _os.reset(new std::ofstream());
    // TODO Optimize this part Does it even matter ?
    std::string log_file_name = _name;
    log_file_name.append(".");
    log_file_name.append(std::to_string(++_file_number));
    log_file_name.append(".log");
    _os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);
  }

 private:
  uint32_t _file_number = 0;
  std::streamoff _bytes_written = 0;
  const uint32_t _log_file_roll_size_bytes;
  const std::string _name;
  std::unique_ptr<std::ofstream> _os;
};

// TODO
class Logger {};

}  // namespace mengsen_log

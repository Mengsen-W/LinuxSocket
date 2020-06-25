/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-05 21:07:13
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-25 19:12:44
 */

#include "log.h"

#include <atomic>   //for atomic and atomic_flag
#include <chrono>   // for time stamp
#include <cstring>  // for memcpy() strlen()
#include <fstream>  // for FileWriter class
#include <queue>    // QueueBuffer class
#include <thread>   // for thread

// internal linkage and means internal visible
namespace {

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
  sprintf(microseconds, "%06lu", timestamp % 1000000);
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
 * Gets the index location of the data type specified
 * in the tuple parameter index
 */
template <typename T, typename Tuple>
struct TupleIndex;
// exit
template <typename T, typename... Types>
struct TupleIndex<T, std::tuple<T, Types...>> {
  static constexpr const std::size_t value = 0;
};
// entrance
template <typename T, typename U, typename... Types>
struct TupleIndex<T, std::tuple<U, Types...>> {
  static constexpr const std::size_t value =
      1 + TupleIndex<T, std::tuple<Types...>>::value;
};

}  // anonymous namespace

namespace mengsen_log {

// for test entrance type
typedef std::tuple<char, uint32_t, uint64_t, int32_t, int64_t, double,
                   LogLine::string_literal_t, char*>
    SupportedTypes;

/**
 * @brief: convert int8_t enum class LogLevel to string
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
    LEVEL(WARN)
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
 * @brief: get a usable buffer sit from stack or heap
 * @param: void
 * @return: char *
 */
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
  // get requited size from used bytes add additional bytes
  const size_t required_size = _bytes_used + additional_bytes;

  // no need resize buffer size
  if (required_size <= _buffer_size) return;

  // need to resize buffer

  if (!_heap_buffer) {
    // no heap space
    _buffer_size = std::max(static_cast<size_t>(512), required_size);
    // new heap
    _heap_buffer.reset(new char[_buffer_size]);
    // init heap
    memcpy(_heap_buffer.get(), _stack_buffer, _bytes_used);
    return;
  } else {
    // has heap space copy and swap
    _buffer_size = std::max(static_cast<size_t>(512), required_size);
    std::unique_ptr<char[]> new_heap_buffer(new char[_buffer_size]);
    memcpy(new_heap_buffer.get(), _heap_buffer.get(), _bytes_used);
    _heap_buffer.swap(new_heap_buffer);
  }
  return;
}

/**
 * @brief: encode arg to buffer for LogLine construction
 * @param:[in] typename Arg arg
 * @return: void
 */
template <typename Arg>
void LogLine::encode(Arg arg) {
  *(reinterpret_cast<Arg*>(buffer())) = arg;
  _bytes_used += sizeof(Arg);
}

/**
 * @brief: call resize_buffer_if_needed() and used in operator <<
 * internal call single encode
 * @param:[in] Arg arg
 * @param:[in] uint8_t type_id
 * @return: void
 */
template <typename Arg>
void LogLine::encode(Arg arg, uint8_t type_id) {
  resize_buffer_if_needed(sizeof(Arg) + sizeof(uint8_t));
  encode<uint8_t>(type_id);
  encode<Arg>(arg);
}

/**
 * @brief: tansform const cha* to string and encode
 * @param: [in] const char *arg
 * @param: [in] size_t length
 * @return: void
 */
void LogLine::encode_c_string(const char* arg, size_t length) {
  if (length == 0) return;
  // There is a space between the type_id and the content
  resize_buffer_if_needed(1 + length + 1);
  char* b = buffer();
  size_t type_id = TupleIndex<char*, SupportedTypes>::value;
  // written type_id
  *(reinterpret_cast<uint8_t*>(b++)) = static_cast<uint8_t>(type_id);
  // copy char * and There's '/0' at the end
  memcpy(b, arg, length + 1);
  _bytes_used += 1 + length + 1;
  return;
}

/**
 * @brief: call encode_c_string and Use strlen() to calculate the length
 * @param: [in] const char *arg
 * @return: void
 */
void LogLine::encode(const char* arg) {
  if (arg != nullptr) encode_c_string(arg, strlen(arg));
  return;
}

/**
 * @brief: call encode_c_string and Use strlen() to calculate the length
 * @param: [in] char *arg
 * @return: void
 */
void LogLine::encode(char* arg) {
  if (arg != nullptr) encode_c_string(arg, strlen(arg));
  return;
}

/**
 * @brief: write os according to type id
 * @param: [in] std::ostream &os
 * @param: [in] char *b
 * @param: [in] typename Arg *dummy (just used to check 'b' type)
 * @return: char *
 */
template <typename Arg>
char* decode(std::ostream& os, char* b, Arg* dummy) {
  Arg arg = *(reinterpret_cast<Arg*>(b));
  os << arg;
  return b + sizeof(Arg);
}

/**
 * @brief: decode All the specialized
 * @param: [in] std::ostream &os
 * @param: [in] char *b
 * @param: [in] LogLine::string_literal_t *dummy
 * @return: char *
 */
template <>
char* decode(std::ostream& os, char* b, LogLine::string_literal_t* dummy) {
  LogLine::string_literal_t s =
      *(reinterpret_cast<LogLine::string_literal_t*>(b));
  os << s._s;
  return b + sizeof(LogLine::string_literal_t);
}

/**
 * @brief: decode All the specialized
 * @param: [in] std::ostream &os
 * @param: [in] char *b
 * @param: [in] char **dummy
 * @return: char *
 */
template <>
char* decode(std::ostream& os, char* b, char** dummy) {
  while (*b != '\0') {
    os << *b;
    ++b;
  }
  return ++b;
}
/**
 * @brief: Provide an interface to buffer write
 * @param: [in] std::ostream &os
 * @return: void
 */
void LogLine::stringify(std::ostream& os) {
  // get space pointer
  char* b = !_heap_buffer ? _stack_buffer : _heap_buffer.get();

  // temp variable 'const char * const end'
  // for stringify() double parameter version
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

  // format timestamp and write to os
  format_timestamp(os, timestamp);

  // write head line
  // timestamp [loglevel][threadid][filename:functionname:line]
  os << '[' << to_string(loglevel) << ']' << '[' << threadid << ']' << '['
     << file._s << ':' << function._s << ':' << line << "] ";

  stringify(os, b, end);
  os << std::endl;

  if (loglevel >= LogLevel::CRIT) {
    // Emergency log immediately output
    os.flush();
  }
  return;
}

/**
 * @brief: recursive write comment according to type id
 * @param: [in] std::ostream &os
 * @param: [in] char *start
 * @param: [in] const char * const end
 * @return: void
 */
void LogLine::stringify(std::ostream& os, char* start, const char* const end) {
  if (start == end) return;
  uint8_t type_id = *(reinterpret_cast<uint8_t*>(start));
  start += sizeof(uint8_t);

  switch (type_id) {
#define CASE(num)                                                           \
  case num:                                                                 \
    stringify(                                                              \
        os,                                                                 \
        decode(os, start,                                                   \
               static_cast<std::tuple_element<num, SupportedTypes>::type*>( \
                   nullptr)),                                               \
        end);                                                               \
    return;

    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    CASE(5)
    CASE(6)
    CASE(7)

#undef CASE
  }
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

LogLine& LogLine::operator<<(const YAML::Node& node) {
  return operator<<(node.as<std::string>());
}

/**
 * @brief: Buffer Base virtual
 */
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
 * @brief: logline's buffer
 */
class Buffer {
 public:
  /**
   * @brief: logline item 256 bytes
   */
  struct Item {
    Item(LogLine&& logline) : _logline(std::move(logline)) {}
    // memory pad
    char padding[256 - sizeof(LogLine)];
    LogLine _logline;
  };

  // 8MB. Helps reduce mempry fragmentation
  static constexpr const size_t size = 32768;

  Buffer() : _buffer(static_cast<Item*>(std::malloc(size * sizeof(Item)))) {
    for (size_t i = 0; i <= size; ++i) {
      // init write_state
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
    // replace new
    new (&_buffer[write_index]) Item(std::move(logline));
    // recorder state
    _write_state[write_index].store(1, std::memory_order_release);
    // save item buffer number
    return _write_state[size].fetch_add(1, std::memory_order_acquire) + 1 ==
           size;
  }

  /**
   * @brief: try pop
   * @param: [out] LogLine &logline
   * @param: [int] const unsigned int read_index
   * @return: bool with success
   */
  bool try_pop(LogLine& logline, const unsigned int read_index) {
    if (_write_state[read_index].load(std::memory_order_acquire)) {
      Item& item = _buffer[read_index];
      logline = std::move(item._logline);
      return true;
    }
    return false;
  }

 private:
  // logline item array
  Item* _buffer;
  // recorder Item state last recorder sum of item
  std::atomic<unsigned int> _write_state[size + 1];
};

/**
 * @brief: Queue buffer management buffer class
 */
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

  /**
   * @brief: push logline to buffer
   * @param: [in] LogLine&& logline
   * @return: void
   */
  void push(LogLine&& logline) override {
    // updare write index
    unsigned int write_index =
        _write_index.fetch_add(1, std::memory_order_relaxed);

    if (write_index < Buffer::size) {
      // have memory left
      if (_current_write_buffer.load(std::memory_order_acquire)
              ->push(std::move(logline), write_index)) {
        // if push() return true means need next buffer
        setup_next_write_buffer();
      }
    } else {
      // wait another thread setup_next_write_buffer()
      while (_write_index.load(std::memory_order_acquire) >= Buffer::size)
        ;
      push(std::move(logline));
    }
  }

  /**
   * @brief: try pop logline
   * @param: [out] LogLine &logline
   * @return: bool with success
   */
  bool try_pop(LogLine& logline) override {
    if (_current_read_buffer == nullptr)
      _current_read_buffer = get_next_read_buffer();

    Buffer* read_buffer = _current_read_buffer;
    if (read_buffer == nullptr) return false;

    if (read_buffer->try_pop(logline, _read_index)) {
      // update read index
      ++_read_index;
      if (_read_index == Buffer::size) {
        // nothing logline need read
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
  /**
   * @brief: set up next buffer
   * @param: void
   * @return: void
   */
  void setup_next_write_buffer() {
    std::unique_ptr<Buffer> next_write_buffer(new Buffer);
    // update current buffer pointer
    _current_write_buffer.store(next_write_buffer.get(),
                                std::memory_order_release);
    SpinLock spinlock(_flags);
    // push new buffer to queue
    _buffers.push(std::move(next_write_buffer));
    // reset write index
    _write_index.store(0, std::memory_order_relaxed);
  }

  /**
   * @brief: get the latest read buffer or front queue
   * @param: void
   * @return: void
   */
  Buffer* get_next_read_buffer() {
    SpinLock spinlock(_flags);
    return _buffers.empty() ? nullptr : _buffers.front().get();
  }

 private:
  // unique_ptr of buffer's queue
  std::queue<std::unique_ptr<Buffer>> _buffers;
  // current write buffer pointer
  std::atomic<Buffer*> _current_write_buffer;
  // current read buffer pointer
  Buffer* _current_read_buffer;
  // write index
  std::atomic<unsigned int> _write_index;
  // atomic flag
  std::atomic_flag _flags;
  // read index
  unsigned int _read_index;
};

/**
 * @brief Multi Producer Single Consumer Ring Buffer
 */
class RingBuffer : public BufferBase {
 public:
  /**
   * @brief: buffer Item
   */
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
      // replace new
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

  /**
   * @brief: push logline override
   * @param: [in] LogLine&& logline
   * @return: void
   */
  void push(LogLine&& logline) override {
    // update write index
    size_t write_index =
        _write_index.fetch_add(1, std::memory_order_relaxed) % _size;
    // get item and write logline to item
    Item& item = _ring[write_index];
    SpinLock spinlock(item.flag);
    item.logline = std::move(logline);
    item.written = 1;
  }

  /**
   * @brief: try pop logline override
   * @param: [out] LogLine &logline
   * @return: bool with success
   */
  bool try_pop(LogLine& logline) override {
    Item& item = _ring[_read_index % _size];
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
  // size of item
  const size_t _size;
  // item array pointer
  Item* _ring;
  // write index size == 8
  std::atomic<size_t> _write_index;
  // memory pad
  char pad[64];
  // read index
  unsigned int _read_index;
};

/**
 * @brief: file writer
 */
class FileWriter {
 public:
  FileWriter(const std::string& log_directorary,
             const std::string& log_file_name, uint32_t log_file_roll_size_mb)
      : _log_file_roll_size_bytes(log_file_roll_size_mb + 1024 * 1024),
        _name(log_directorary + log_file_name) {
    roll_file();
  }

  /**
   * @brief: write log line to log file
   * @param: [in] LogLine& logline
   * @return: void
   */
  void write(LogLine& logline) {
    // get pos of output stream
    std::streampos pos = _os->tellp();
    // write in output stream
    logline.stringify(*_os);
    // save written position
    _bytes_written += _os->tellp() - pos;
    // check roll file
    if (_bytes_written > _log_file_roll_size_bytes) roll_file();
    return;
  }

 private:
  /**
   * @brief: roll file
   * @param: void
   * @return: void
   */
  void roll_file() {
    // clean and close output stream
    if (_os) {
      _os->flush();
      _os->close();
    }
    // clear written bytes
    _bytes_written = 0;
    // reset os pointer
    _os.reset(new std::ofstream());
    // TODO Optimize this part Does it even matter ?
    std::string log_file_name = _name;
    log_file_name.append(".");
    log_file_name.append(std::to_string(++_file_number));
    log_file_name.append(".log");
    // open file with output stream
    _os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);
  }

 private:
  // save file number of written
  uint32_t _file_number = 0;
  // Type to represent position offsets in a stream
  std::streamoff _bytes_written = 0;
  // roll file size
  const uint32_t _log_file_roll_size_bytes;
  // file name
  const std::string _name;
  // output file stream
  std::unique_ptr<std::ofstream> _os;
};

/**
 * @brief: multi-thread logger
 */
class Logger {
 public:
  /**
   * @brief: constructor with NonGuaranteedLogger
   */
  Logger(NonGuaranteedLogger ngl, const std::string& log_directorary,
         const std::string& log_file_name, uint32_t log_file_roll_size_mb)
      : _state(State::INIT),
        _buffer_base(
            new RingBuffer(std::max(1u, ngl._ring_buffer_size_mb) * 1024 * 4)),
        _file_writer(log_directorary, log_file_name,
                     std::max(1u, log_file_roll_size_mb)),
        _thread(&Logger::pop, this) {
    _state.store(State::READY, std::memory_order_release);
  }

  /**
   * @brief: constructor with GuaranteedLogger
   */
  Logger(GuaranteedLogger gl, const std::string& log_directorary,
         const std::string& log_file_name, uint32_t log_file_roll_size_mb)
      : _state(State::INIT),
        _buffer_base(new QueueBuffer),
        _file_writer(log_directorary, log_file_name,
                     std::max(1u, log_file_roll_size_mb)),
        _thread(&Logger::pop, this) {
    _state.store(State::READY, std::memory_order_release);
  }

  ~Logger() {
    _state.store(State::SHUTDOWN);
    _thread.join();
  }

  /**
   * @brief: push logline to buffer
   * @param: [in] LogLine&& logline
   * @return: void
   */
  void add(LogLine&& logline) { _buffer_base->push(std::move(logline)); }

  /**
   * @brief: another thread just pop
   */
  void pop() {
    // wait for constructor to complete
    // and pull all stores done there to thisthread / core.
    while (_state.load(std::memory_order_acquire) == State::INIT)
      std::cout << "INIT" << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    LogLine logline(LogLevel::INFO, nullptr, nullptr, 0);

    // update State to READY
    while (_state.load() == State::READY) {
      std::cout << "ready" << std::endl;
      if (_buffer_base->try_pop(logline))
        // pop logline and write to file
        _file_writer.write(logline);
      else
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // State::SHUTDOWN
    // pop and log all remaining entries
    while (_buffer_base->try_pop(logline)) _file_writer.write(logline);
  }

 private:
  // logger state
  enum class State { INIT, READY, SHUTDOWN };
  // atomic state
  std::atomic<State> _state;
  // management bufferbase class
  std::unique_ptr<BufferBase> _buffer_base;
  // file writer class
  FileWriter _file_writer;
  // thread
  std::thread _thread;
};

// static variable
// unique_ptr to logger
static std::unique_ptr<Logger> logger;
// atomic Logger*
static std::atomic<Logger*> atomic_logger;

/**
 * @brief: initialize atomic_logger and add logline
 * @param: [in] LogLine& logline
 * @return: bool with success
 */
bool Log::operator==(LogLine& logline) {
  atomic_logger.load(std::memory_order_acquire)->add(std::move(logline));
  return true;
}

void initialize(NonGuaranteedLogger ngl, const std::string& log_directorary,
                const std::string& log_file_name,
                uint32_t log_file_roll_size_mb) {
  logger.reset(
      new Logger(ngl, log_directorary, log_file_name, log_file_roll_size_mb));
  atomic_logger.store(logger.get(), std::memory_order_seq_cst);
}

void initialize(GuaranteedLogger gl, const std::string& log_directorary,
                const std::string& log_file_name,
                uint32_t log_file_roll_size_mb) {
  logger.reset(
      new Logger(gl, log_directorary, log_file_name, log_file_roll_size_mb));
  atomic_logger.store(logger.get(), std::memory_order_seq_cst);
}

// defaule DEBUG loglevel
std::atomic<unsigned int8_t> loglevel = {0};

void set_log_level(LogLevel level) {
  loglevel.store(static_cast<unsigned int>(level), std::memory_order_release);
}

bool is_logged(LogLevel level) {
  return static_cast<unsigned int>(level) >=
         loglevel.load(std::memory_order_relaxed);
}

}  // namespace mengsen_log

/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-13 17:55:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-13 20:31:07
 */

#include <iostream>

#include "log.h"

void testlog() {
  // Ensure initialize is called once prior to logging.
  // This will create log files like /tmp/mengsen_log1.txt,
  // /tmp/mengsen_log2.txt etc. Log will roll to the next file after every 1MB.
  // This will initialize the guaranteed logger.
  mengsen_log::initialize(mengsen_log::GuaranteedLogger(), "./", "mengsen_log",
                          1);

  // Or if you want to use the non guaranteed logger -
  // ring_buffer_size_mb - LogLines are pushed into a mpsc ring buffer whose
  // size is determined by this parameter. Since each LogLine is 256 bytes,
  // ring_buffer_size = ring_buffer_size_mb * 1024 * 1024 / 256
  // In this example ring_buffer_size_mb = 3.
  // mengsen_log::initialize(mengsen_log::NonGuaranteedLogger(3), "/tmp/",
  // "mengsen_log", 1);

  // for (int i = 0; i < 1; ++i) {
  LOG_INFO << "Sample mengsen_log";
  // }

  // Change log level at run-time.
  // mengsen_log::set_log_level(mengsen_log::LogLevel::CRIT);
  // LOG_WARN
  //     << "This log line will not be logged since we are at loglevel =
  //     CRIT";
}

int main() {
  testlog();
  LOG_INFO << "main";
  return 0;
}
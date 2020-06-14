/*
 * @Author: Mengsen.Wang
 * @Date: 2020-06-13 17:55:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-06-13 20:31:07
 */

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

#include "log.h"

/* Returns microseconds since epoch */
uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() /
         std::chrono::microseconds(1);
}

void mengsen_log_benchmark() {
  int const iterations = 100000;
  char const* const benchmark = "benchmark";
  uint64_t begin = timestamp_now();
  for (int i = 0; i < iterations; ++i)
    LOG_INFO << "Logging " << benchmark << i << 0 << 'K' << -42.42;
  uint64_t end = timestamp_now();
  long int avg_latency = (end - begin) * 1000 / iterations;
  printf("\tAverage mengsen_log Latency = %ld nanoseconds\n", avg_latency);
}

template <typename Function>
void run_benchmark(Function&& f, int thread_count) {
  printf("Thread count: %d\n", thread_count);
  std::vector<std::thread> threads;
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back(f);
  }
  for (int i = 0; i < thread_count; ++i) {
    threads[i].join();
  }
}

int main() {
  // Ring buffer size is passed as 10 mega bytes.
  // Since each log line = 256 bytes, thats 40960 slots.
  mengsen_log::initialize(mengsen_log::NonGuaranteedLogger(10),
                          "/home/Mengsen/LinuxSocket/mengsen/log",
                          "mengsen_log", 1);
  for (auto threads : {1, 2, 3, 4, 5})
    run_benchmark(mengsen_log_benchmark, threads);

  return 0;
}

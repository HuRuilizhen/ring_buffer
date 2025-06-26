#include <chrono>
#include <iostream>
#include <thread>

#include "ring_buffer/ring_buffer_mutex.h"

using namespace std::chrono;

constexpr int ITERATIONS = 1000000;
constexpr int CAPACITY = 1024;

void benchmark_mutex() {
  RingBuffer::RingBufferMutex<int> buffer(CAPACITY);
  auto start = high_resolution_clock::now();

  std::thread producer([&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        try {
          buffer.push(i);
          break;
        } catch (const std::overflow_error &) {
          std::this_thread::yield();
        }
      }
    }
  });

  std::thread consumer([&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        try {
          buffer.pop();
          break;
        } catch (const std::underflow_error &) {
          std::this_thread::yield();
        }
      }
    }
  });

  producer.join();
  consumer.join();

  auto end = high_resolution_clock::now();
  std::chrono::duration<double> dur = end - start;
  std::cout << "RingBufferMutex SPSC time: " << dur.count() << " s"
            << std::endl;
}

#include <chrono>
#include <iostream>
#include <thread>

#include "ring_buffer/mpsc_ring_buffer.h"

using namespace std::chrono;

constexpr int ITERATIONS = 1000000;
constexpr int CAPACITY = 1024;

void benchmark_mpsc() {
  RingBuffer::MPSCRingBuffer<int> buffer(CAPACITY);
  auto start = high_resolution_clock::now();

  std::thread producer([&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        if (buffer.tryPush(i)) break;
        std::this_thread::yield();
      }
    }
  });

  std::thread consumer([&]() {
    int value;
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        if (buffer.tryPop(value)) break;
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  auto end = high_resolution_clock::now();
  std::chrono::duration<double> dur = end - start;
  std::cout << "MPSC RingBuffer SPSC time: " << dur.count() << " s"
            << std::endl;
}

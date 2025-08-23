#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "internal/common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) SPSCRingBuffer {
  std::unique_ptr<T[], Common::AlignedDeleter> buffer;
  const size_t capacity;

  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> head = 0;
  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> tail = 0;

 public:
  explicit SPSCRingBuffer<T>(size_t cap,
                             std::size_t align = Common::CACHELINE_SIZE);

  bool tryPush(const T& value);
  bool tryPush(T&& value);
  bool tryPop(T& value);
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
SPSCRingBuffer<T>::SPSCRingBuffer(size_t cap, std::size_t align)
    : capacity(cap + 1), buffer(nullptr, Common::AlignedDeleter{align}) {
  if (cap == 0)
    throw std::invalid_argument(
        "capacity of ring buffer must be greater than 0");

  T* ptr = static_cast<T*>(
      ::operator new[](capacity * sizeof(T), std::align_val_t(align)));
  buffer.reset(ptr);
}

template <typename T>
bool SPSCRingBuffer<T>::tryPush(const T& value) {
  const size_t current_tail = tail.load(std::memory_order_relaxed);
  const size_t next_tail = (current_tail + 1) % capacity;

  if (next_tail == head.load(std::memory_order_acquire)) return false;

  buffer[current_tail] = value;
  tail.store(next_tail, std::memory_order_release);
  return true;
}

template <typename T>
bool SPSCRingBuffer<T>::tryPush(T&& value) {
  const size_t current_tail = tail.load(std::memory_order_relaxed);
  const size_t next_tail = (current_tail + 1) % capacity;

  if (next_tail == head.load(std::memory_order_acquire)) return false;

  buffer[current_tail] = std::move(value);
  tail.store(next_tail, std::memory_order_release);
  return true;
}

template <typename T>
bool SPSCRingBuffer<T>::tryPop(T& value) {
  const size_t current_head = head.load(std::memory_order_relaxed);

  if (current_head == tail.load(std::memory_order_acquire)) return false;

  value = std::move(buffer[current_head]);
  head.store((current_head + 1) % capacity, std::memory_order_release);
  return true;
}

}  // namespace RingBuffer

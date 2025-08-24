#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) RingBufferMutex {
 private:
  std::unique_ptr<T[], Common::AlignedDeleter> buffer;
  const size_t capacity;

  alignas(Common::CACHELINE_SIZE) mutable std::mutex mtx;
  alignas(Common::CACHELINE_SIZE) size_t head = 0;
  alignas(Common::CACHELINE_SIZE) size_t tail = 0;

 public:
  explicit RingBufferMutex(size_t cap,
                           std::size_t align = Common::CACHELINE_SIZE);

  bool tryPush(const T& value);
  bool tryPush(T&& value);
  bool tryPop(T& value);
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
RingBufferMutex<T>::RingBufferMutex(size_t cap, std::size_t align)
    : capacity(cap + 1), buffer(nullptr, Common::AlignedDeleter{align}) {
  if (cap == 0)
    throw std::invalid_argument(
        "capacity of ring buffer must be greater than 0");

  T* ptr = static_cast<T*>(
      ::operator new[](capacity * sizeof(T), std::align_val_t(align)));
  buffer.reset(ptr);
}

template <typename T>
bool RingBufferMutex<T>::tryPush(const T& value) {
  std::lock_guard<std::mutex> lock(mtx);
  if (((tail + 1) % capacity) == head) return false;
  buffer[tail] = value;
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool RingBufferMutex<T>::tryPush(T&& value) {
  std::lock_guard<std::mutex> lock(mtx);
  if (((tail + 1) % capacity) == head) return false;
  buffer[tail] = std::move(value);
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool RingBufferMutex<T>::tryPop(T& value) {
  std::lock_guard<std::mutex> lock(mtx);
  if (head == tail) return false;
  value = std::move(buffer[head]);
  head = (head + 1) % capacity;
  return true;
}

}  // namespace RingBuffer

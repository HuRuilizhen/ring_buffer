#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace RingBuffer {  // interface

template <typename T>
class BasicRingBuffer {
 private:
  std::unique_ptr<T[]> buffer;
  const size_t capacity;
  size_t head = 0;
  size_t tail = 0;

 public:
  explicit BasicRingBuffer(size_t cap);
  bool tryPush(const T &value);
  bool tryPush(T &&value);
  template <typename... Args>
  bool tryEmplace(Args &&...args);
  bool tryPop(T &value);
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
BasicRingBuffer<T>::BasicRingBuffer(size_t cap)
    : capacity(cap + 1), buffer(std::make_unique<T[]>(cap + 1)) {
  if (cap == 0)
    throw std::invalid_argument(
        "capacity of ring buffer must be greater than 0");
}

template <typename T>
bool BasicRingBuffer<T>::tryPush(const T &value) {
  if ((tail + 1) % capacity == head) return false;
  buffer[tail] = value;
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool BasicRingBuffer<T>::tryPush(T &&value) {
  if ((tail + 1) % capacity == head) return false;
  buffer[tail] = std::move(value);
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
template <typename... Args>
bool BasicRingBuffer<T>::tryEmplace(Args &&...args) {
  if ((tail + 1) % capacity == head) return false;
  new (&buffer[tail]) T(std::forward<Args>(args)...);
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool BasicRingBuffer<T>::tryPop(T &value) {
  if (head == tail) return false;

  T *elem = &buffer[head];
  value.~T();
  new (&value) T(std::move(*elem));
  elem->~T();

  head = (head + 1) % capacity;
  return true;
}

}  // namespace RingBuffer

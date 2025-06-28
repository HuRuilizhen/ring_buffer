#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>

#include "common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) RingBufferMutex {
 private:
  std::unique_ptr<T[], Common::AlignedDeleter> buffer;
  const size_t capacity;
  const std::size_t alignment;

  alignas(Common::CACHELINE_SIZE) mutable std::mutex mtx;
  alignas(Common::CACHELINE_SIZE) size_t head = 0;
  alignas(Common::CACHELINE_SIZE) size_t tail = 0;

 public:
  explicit RingBufferMutex(size_t cap,
                           std::size_t align = Common::CACHELINE_SIZE);

  void push(const T& value);
  void push(T&& value);
  T pop();

  bool isFull() const;
  bool isEmpty() const;
  size_t getCapacity() const;
  size_t getSize() const;
  void print() const;
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
RingBufferMutex<T>::RingBufferMutex(size_t cap, std::size_t align)
    : capacity(cap + 1),
      alignment(align),
      buffer(nullptr, Common::AlignedDeleter{align}) {
  if (cap == 0)
    throw std::invalid_argument(
        "capacity of ring buffer must be greater than 0");

  T* ptr = static_cast<T*>(
      ::operator new[](capacity * sizeof(T), std::align_val_t(align)));
  buffer.reset(ptr);
}

template <typename T>
void RingBufferMutex<T>::push(const T& value) {
  std::lock_guard<std::mutex> lock(mtx);
  if (isFull()) throw std::overflow_error("ring buffer is full");
  buffer[tail] = value;
  tail = (tail + 1) % capacity;
}

template <typename T>
void RingBufferMutex<T>::push(T&& value) {
  std::lock_guard<std::mutex> lock(mtx);
  if (isFull()) throw std::overflow_error("ring buffer is full");
  buffer[tail] = std::move(value);
  tail = (tail + 1) % capacity;
}

template <typename T>
T RingBufferMutex<T>::pop() {
  std::lock_guard<std::mutex> lock(mtx);
  if (isEmpty()) throw std::underflow_error("ring buffer is empty");
  T value = std::move(buffer[head]);
  head = (head + 1) % capacity;
  return value;
}

template <typename T>
bool RingBufferMutex<T>::isFull() const {
  return ((tail + 1) % capacity) == head;
}

template <typename T>
bool RingBufferMutex<T>::isEmpty() const {
  return head == tail;
}

template <typename T>
size_t RingBufferMutex<T>::getCapacity() const {
  return capacity - 1;
}

template <typename T>
size_t RingBufferMutex<T>::getSize() const {
  return (tail + capacity - head) % capacity;
}

template <typename T>
void RingBufferMutex<T>::print() const {
  std::lock_guard<std::mutex> lock(mtx);
  const size_t count = (tail + capacity - head) % capacity;
  for (size_t i = 0; i < count; ++i)
    std::cout << buffer[(head + i) % capacity] << " ";
  std::cout << std::endl;
}

}  // namespace RingBuffer

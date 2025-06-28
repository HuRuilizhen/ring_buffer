#pragma once

#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) RingBufferAtomic {
  std::unique_ptr<T[], Common::AlignedDeleter> buffer;
  const size_t capacity;
  const std::size_t alignment;

  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> head = 0;
  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> tail = 0;

 public:
  explicit RingBufferAtomic<T>(size_t cap,
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
RingBufferAtomic<T>::RingBufferAtomic(size_t cap, std::size_t align)
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
void RingBufferAtomic<T>::push(const T& value) {
  const size_t current_tail = tail.load(std::memory_order_relaxed);
  const size_t next_tail = (current_tail + 1) % capacity;

  if (next_tail == head.load(std::memory_order_acquire))
    throw std::overflow_error("ring buffer is full");

  buffer[current_tail] = value;
  tail.store(next_tail, std::memory_order_release);
}

template <typename T>
void RingBufferAtomic<T>::push(T&& value) {
  const size_t current_tail = tail.load(std::memory_order_relaxed);
  const size_t next_tail = (current_tail + 1) % capacity;

  if (next_tail == head.load(std::memory_order_acquire))
    throw std::overflow_error("ring buffer is full");

  buffer[current_tail] = std::move(value);
  tail.store(next_tail, std::memory_order_release);
}

template <typename T>
T RingBufferAtomic<T>::pop() {
  const size_t current_head = head.load(std::memory_order_relaxed);

  if (current_head == tail.load(std::memory_order_acquire))
    throw std::underflow_error("ring buffer is empty");

  T value = std::move(buffer[current_head]);
  head.store((current_head + 1) % capacity, std::memory_order_release);
  return value;
}

template <typename T>
bool RingBufferAtomic<T>::isFull() const {
  const size_t next_tail =
      (tail.load(std::memory_order_relaxed) + 1) % capacity;
  return next_tail == head.load(std::memory_order_acquire);
}

template <typename T>
bool RingBufferAtomic<T>::isEmpty() const {
  return head.load(std::memory_order_acquire) ==
         tail.load(std::memory_order_acquire);
}

template <typename T>
size_t RingBufferAtomic<T>::getCapacity() const {
  return capacity - 1;
}

template <typename T>
size_t RingBufferAtomic<T>::getSize() const {
  size_t current_head = head.load(std::memory_order_acquire);
  size_t current_tail = tail.load(std::memory_order_acquire);
  return (current_tail + capacity - current_head) % capacity;
}

template <typename T>
void RingBufferAtomic<T>::print() const {
  const size_t size = getSize();
  const size_t current_head = head.load(std::memory_order_acquire);
  for (size_t i = 0; i < size; i++)
    std::cout << buffer[(current_head + i) % capacity] << " ";
  std::cout << std::endl;
}

}  // namespace RingBuffer

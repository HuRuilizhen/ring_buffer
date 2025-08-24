#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace RingBuffer {  // interface

template <typename T>
class RingBuffer {
 private:
  std::unique_ptr<T[]> buffer;
  const size_t capacity;
  size_t head = 0;
  size_t tail = 0;

 public:
  explicit RingBuffer(size_t cap);
  bool tryPush(const T &value);
  bool tryPush(T &&value);
  bool tryPop(T &value);

  bool isFull() const;
  bool isEmpty() const;
  size_t getCapacity() const;
  size_t getSize() const;
  void print() const;
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
RingBuffer<T>::RingBuffer(size_t cap)
    : capacity(cap + 1), buffer(std::make_unique<T[]>(cap + 1)) {
  if (cap == 0)
    throw std::invalid_argument(
        "capacity of ring buffer must be greater than 0");
}

template <typename T>
bool RingBuffer<T>::tryPush(const T &value) {
  if (isFull()) return false;
  buffer[tail] = value;
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool RingBuffer<T>::tryPush(T &&value) {
  if (isFull()) return false;
  buffer[tail] = std::move(value);
  tail = (tail + 1) % capacity;
  return true;
}

template <typename T>
bool RingBuffer<T>::tryPop(T &value) {
  if (isEmpty()) return false;
  value = std::move(buffer[head]);
  head = (head + 1) % capacity;
  return true;
}

template <typename T>
bool RingBuffer<T>::isFull() const {
  const size_t next_tail = (tail + 1) % capacity;
  return next_tail == head;
}

template <typename T>
bool RingBuffer<T>::isEmpty() const {
  return head == tail;
}

template <typename T>
size_t RingBuffer<T>::getCapacity() const {
  return capacity - 1;
}

template <typename T>
size_t RingBuffer<T>::getSize() const {
  return (tail + capacity - head) % capacity;
}

template <typename T>
void RingBuffer<T>::print() const {
  const size_t size = getSize();
  for (size_t i = 0; i < size; i++)
    std::cout << buffer[(head + i) % capacity] << " ";
  std::cout << std::endl;
}

}  // namespace RingBuffer

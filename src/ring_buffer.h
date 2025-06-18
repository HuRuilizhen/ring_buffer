#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>

namespace RingBuffer {

template <typename T>
class RingBuffer {
 private:
  std::unique_ptr<T[]> buffer;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;

 public:
  explicit RingBuffer(size_t capacity);
  ~RingBuffer() = default;

  void push(const T &value);
  T pop();

  bool isEmpty() const;
  bool isFull() const;
  size_t getSize() const;
  size_t getCapacity() const;

  void print() const;
  std::span<const T> getElements() const;
};

template <typename T>
RingBuffer<T>::RingBuffer(size_t capacity)
    : capacity(capacity),
      size(0),
      head(0),
      tail(0),
      buffer(std::make_unique<T[]>(capacity)) {
  if (capacity == 0)
    throw std::invalid_argument("capacity of ring buffer must greater than 0");
}

template <typename T>
void RingBuffer<T>::push(const T &value) {
  if (isFull()) throw std::overflow_error("ring buffer is full");

  buffer[tail] = value;
  tail = (tail + 1) % capacity;
  size++;
}

template <typename T>
T RingBuffer<T>::pop() {
  if (isEmpty()) throw std::underflow_error("ring buffer is empty");

  T value = std::move(buffer[head]);
  head = (head + 1) % capacity;
  size--;

  return value;
}

template <typename T>
bool RingBuffer<T>::isFull() const {
  return size == capacity;
}

template <typename T>
bool RingBuffer<T>::isEmpty() const {
  return size == 0;
}

template <typename T>
size_t RingBuffer<T>::getCapacity() const {
  return capacity;
}

template <typename T>
size_t RingBuffer<T>::getSize() const {
  return size;
}

template <typename T>
void RingBuffer<T>::print() const {
  for (int i = 0; i < size; i++) std::cout << buffer[head + i] << " ";
  std::cout << std::endl;
}

template <typename T>
std::span<const T> RingBuffer<T>::getElements() const {
  return std::span<const T>(buffer.get(), size);
}

}  // namespace RingBuffer

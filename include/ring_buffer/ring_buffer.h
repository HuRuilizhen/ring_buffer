#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>

namespace RingBuffer {

#ifndef __cpp_lib_hardware_interference_size
constexpr std::size_t CACHELINE_SIZE = 64;  // fallback
#else
constexpr std::size_t CACHELINE_SIZE =
    std::hardware_destructive_interference_size;
#endif

template <typename T>
class alignas(CACHELINE_SIZE) RingBuffer {
 private:
  struct Deleter {
    std::size_t alignment;
    void operator()(T* ptr) const noexcept {
      ::operator delete[](ptr, std::align_val_t(alignment));
    }
  };

  std::unique_ptr<T[], Deleter> buffer;
  const size_t capacity;
  const std::size_t alignment;
  alignas(CACHELINE_SIZE) size_t size = 0;
  alignas(CACHELINE_SIZE) size_t head = 0;
  alignas(CACHELINE_SIZE) size_t tail = 0;

 public:
  explicit RingBuffer(size_t cap, std::size_t align = CACHELINE_SIZE)
      : capacity(cap), alignment(align), buffer(nullptr, Deleter{align}) {
    if (cap == 0)
      throw std::invalid_argument(
          "capacity of ring buffer must be greater than 0");
    T* ptr = static_cast<T*>(
        ::operator new[](cap * sizeof(T), std::align_val_t(align)));
    buffer.reset(ptr);
  }

  void push(const T& value) {
    if (isFull()) throw std::overflow_error("ring buffer is full");

    buffer[tail] = value;
    tail = (tail + 1) % capacity;
    size++;
  }

  T pop() {
    if (isEmpty()) throw std::underflow_error("ring buffer is empty");

    T value = std::move(buffer[head]);
    head = (head + 1) % capacity;
    size--;

    return value;
  }

  bool isFull() const { return size == capacity; }
  bool isEmpty() const { return size == 0; }
  size_t getCapacity() const { return capacity; }
  size_t getSize() const { return size; }

  void print() const {
    for (size_t i = 0; i < size; i++)
      std::cout << buffer[(head + i) % capacity] << " ";
    std::cout << std::endl;
  }

  std::span<const T> getElements() const {
    return std::span<const T>(buffer.get(), size);
  }
};

}  // namespace RingBuffer

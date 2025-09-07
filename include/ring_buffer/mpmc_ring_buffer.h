#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "internal/common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) MPMCRingBuffer {
 private:
  struct Slot {
    alignas(Common::CACHELINE_SIZE) std::atomic<size_t> seq;
    alignas(Common::CACHELINE_SIZE) T data;
  };

  std::unique_ptr<Slot[], Common::AlignedDeleter> buffer;
  const size_t capacity;
  const std::size_t alignment;

  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> tail = 0;
  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> head = 0;

 public:
  explicit MPMCRingBuffer(size_t cap,
                          std::size_t align = Common::CACHELINE_SIZE);
  bool tryPush(const T &value);
  bool tryPush(T &&value);
  template <typename... Args>
  bool tryEmplace(Args &&...args);
  bool tryPop(T &value);
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
MPMCRingBuffer<T>::MPMCRingBuffer(size_t cap, std::size_t align)
    : capacity(cap),
      alignment(align),
      buffer(nullptr, Common::AlignedDeleter{align}) {
  if (cap == 0) throw std::invalid_argument("Capacity must be greater than 0");

  Slot *raw = static_cast<Slot *>(
      ::operator new[](capacity * sizeof(Slot), std::align_val_t(align)));
  for (size_t i = 0; i < capacity; ++i) {
    new (&raw[i]) Slot{.seq = i};  // placement new
  }
  buffer.reset(raw);
}

template <typename T>
bool MPMCRingBuffer<T>::tryPush(const T &value) {
  size_t pos = tail.load(std::memory_order_relaxed);

  while (true) {
    Slot &slot = buffer[pos % capacity];
    size_t expected = pos;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      slot.data = value;
      slot.seq.store(expected + 1, std::memory_order_release);
      return true;
    }
  }
}

template <typename T>
bool MPMCRingBuffer<T>::tryPush(T &&value) {
  size_t pos = tail.load(std::memory_order_relaxed);

  while (true) {
    Slot &slot = buffer[pos % capacity];
    size_t expected = pos;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      slot.data = std::move(value);
      slot.seq.store(expected + 1, std::memory_order_release);
      return true;
    }
  }
}

template <typename T>
template <typename... Args>
bool MPMCRingBuffer<T>::tryEmplace(Args &&...args) {
  size_t pos = tail.load(std::memory_order_relaxed);

  while (true) {
    Slot &slot = buffer[pos % capacity];
    size_t expected = pos;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      new (&slot.data) T(std::forward<Args>(args)...);
      slot.seq.store(expected + 1, std::memory_order_release);
      return true;
    }
  }
}

template <typename T>
bool MPMCRingBuffer<T>::tryPop(T &value) {
  size_t pos = head.load(std::memory_order_relaxed);

  while (true) {
    Slot &slot = buffer[pos % capacity];
    size_t expected = pos + 1;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (head.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      T *elem = &(slot.data);
      value.~T();
      new (&value) T(std::move(*elem));
      elem->~T();

      slot.seq.store(pos + capacity, std::memory_order_release);
      return true;
    }
  }
}

}  // namespace RingBuffer

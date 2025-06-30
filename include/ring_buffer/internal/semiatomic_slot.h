#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "common.h"

namespace RingBuffer {  // interface

template <typename T>
class alignas(Common::CACHELINE_SIZE) RingBufferSemiAtomicSlot {
 private:
  struct Slot {
    alignas(Common::CACHELINE_SIZE) std::atomic<size_t> seq;
    alignas(Common::CACHELINE_SIZE) T data;
  };

  std::unique_ptr<Slot[], Common::AlignedDeleter> buffer;
  const size_t capacity;
  const std::size_t alignment;

  alignas(Common::CACHELINE_SIZE) std::atomic<size_t> tail = 0;
  alignas(Common::CACHELINE_SIZE) size_t head = 0;

 public:
  explicit RingBufferSemiAtomicSlot(size_t cap,
                                    std::size_t align = Common::CACHELINE_SIZE);

  bool tryPush(const T& item);
  bool tryPush(T&& item);
  bool tryPop(T& out);

  size_t getCapacity() const;
  bool isEmpty() const;
};

}  // namespace RingBuffer

namespace RingBuffer {  // implementation

template <typename T>
RingBufferSemiAtomicSlot<T>::RingBufferSemiAtomicSlot(size_t cap,
                                                      std::size_t align)
    : capacity(cap),
      alignment(align),
      buffer(nullptr, Common::AlignedDeleter{align}) {
  if (cap == 0) throw std::invalid_argument("Capacity must be greater than 0");

  Slot* raw = static_cast<Slot*>(
      ::operator new[](capacity * sizeof(Slot), std::align_val_t(align)));
  for (size_t i = 0; i < capacity; ++i) {
    new (&raw[i]) Slot{.seq = i};  // placement new
  }
  buffer.reset(raw);
}

template <typename T>
bool RingBufferSemiAtomicSlot<T>::tryPush(const T& item) {
  size_t pos = tail.load(std::memory_order_relaxed);

  while (true) {
    Slot& slot = buffer[pos % capacity];
    size_t expected = pos;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      slot.data = item;
      slot.seq.store(expected + 1, std::memory_order_release);
      return true;
    }
  }
}

template <typename T>
bool RingBufferSemiAtomicSlot<T>::tryPush(T&& item) {
  size_t pos = tail.load(std::memory_order_relaxed);

  while (true) {
    Slot& slot = buffer[pos % capacity];
    size_t expected = pos;

    if (slot.seq.load(std::memory_order_acquire) != expected) {
      return false;
    }

    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
      slot.data = std::move(item);
      slot.seq.store(expected + 1, std::memory_order_release);
      return true;
    }
  }
}

template <typename T>
bool RingBufferSemiAtomicSlot<T>::tryPop(T& out) {
  Slot& slot = buffer[head % capacity];
  size_t expected = head + 1;

  if (slot.seq.load(std::memory_order_acquire) != expected) return false;

  out = std::move(slot.data);
  slot.seq.store(head + capacity, std::memory_order_release);
  ++head;
  return true;
}

template <typename T>
size_t RingBufferSemiAtomicSlot<T>::getCapacity() const {
  return capacity;
}

template <typename T>
bool RingBufferSemiAtomicSlot<T>::isEmpty() const {
  Slot& slot = buffer[head % capacity];
  return slot.seq.load(std::memory_order_acquire) != head + 1;
}

}  // namespace RingBuffer

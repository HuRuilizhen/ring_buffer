#pragma once

#include <memory>

namespace RingBuffer::Common {

#ifndef __cpp_lib_hardware_interference_size
constexpr std::size_t CACHELINE_SIZE = 64;
#else
constexpr std::size_t CACHELINE_SIZE =
    std::hardware_destructive_interference_size;
#endif

struct AlignedDeleter {
  std::size_t alignment;

  void operator()(void* ptr) const noexcept {
    ::operator delete[](ptr, std::align_val_t(alignment));
  }
};

}  // namespace RingBuffer::Common

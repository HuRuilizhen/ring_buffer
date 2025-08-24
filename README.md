# ring_buffer

![Build Status](https://img.shields.io/github/actions/workflow/status/HuRuilizhen/ring_buffer/cmake-multi-platform.yml?branch=release)

A header-only C++20 ring buffer library providing several synchronization strategies. Unit tests and microbenchmarks are included.

## Features

- Basic, SPSC, MPSC and MPMC variants
- Implementations using mutexes, atomics and semi-atomic slots
- Simple integration through CMake's `find_package`

## Requirements

- C++20 compatible compiler
- CMake >= 3.15
- (Optional) GoogleTest for unit tests

## Table of Contents

- [ring\_buffer](#ring_buffer)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Table of Contents](#table-of-contents)
  - [Building](#building)
    - [Configuration](#configuration)
    - [Execution](#execution)
  - [Runing Benchmarks](#runing-benchmarks)
  - [Running Tests](#running-tests)
  - [Using the Library](#using-the-library)
    - [Including the Library](#including-the-library)
    - [Supported APIs](#supported-apis)
    - [Quick Example](#quick-example)
    - [Project Using the Library](#project-using-the-library)
  - [Uninstall](#uninstall)
  - [Contributing](#contributing)
  - [License](#license)

## Building

### Configuration

| Option                          | Default | Description                               |
| ------------------------------- | ------- | ----------------------------------------- |
| `ENABLE_TESTS`                  | OFF     | Build unit tests (requires GTest)         |
| `ENABLE_BENCHMARKS`             | OFF     | Build microbenchmarks                     |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | OFF     | Generate `compile_commands.json` for IDEs |

### Execution

```bash
# Default build
cmake -S . -B build
cmake --build build

# Build with tests + benchmarks
cmake -S . -B build -DENABLE_TESTS=ON -DENABLE_BENCHMARKS=ON
cmake --build build

# Install
sudo cmake --install build
```

## Runing Benchmarks

```bash
cd ./build/bin && ./ring_buffer_benchmark
```

## Running Tests

```bash
cd ./build && ctest --output-on-failure
```

## Using the Library

### Including the Library

After installation or fetching you can import the target in your own project:

```cmake
find_package(ring_buffer QUIET)

if(ring_buffer_FOUND)
  message(STATUS "Found local ring_buffer: using ring_buffer::ring_buffer")
else()
  message(STATUS "ring_buffer not found locally; fetching via FetchContent")

  include(FetchContent)
  FetchContent_Declare(
    ring_buffer
    GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
    GIT_TAG v1.0.2)  # latest stable version
  FetchContent_MakeAvailable(ring_buffer)
endif()

add_library(your_target STATIC src/your_target.cc)
target_include_directories(
  your_target PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:include>)
target_link_libraries(
  your_target PUBLIC $<$<BOOL:${ring_buffer_FOUND}>:ring_buffer::ring_buffer>
                      $<$<NOT:$<BOOL:${ring_buffer_FOUND}>>:ring_buffer>)
```

### Supported APIs

| Class                | Model | Synchronization           |
| -------------------- | ----- | ------------------------- |
| `BasicRingBuffer<T>` | Basic | None (single-thread only) |
| `SPSCRingBuffer<T>`  | SPSC  | atomics                   |
| `MPSCRingBuffer<T>`  | MPSC  | atomics + slots           |
| `MPMCRingBuffer<T>`  | MPMC  | mutex                     |

**Common API** (all variants):

```cpp
bool tryPush(const T& value);
bool tryPush(T&& value);
template <typename... Args>
bool tryEmplace(Args&&... args);
bool tryPop(T& value);
```

### Quick Example

```cpp
#include <ring_buffer/basic_ring_buffer.h>
#include <iostream>

int main() {
  RingBuffer::BasicRingBuffer<int> rb(3);

  rb.tryEmplace(1);
  rb.tryPush(2);
  rb.tryPush(3);

  int x;
  while (rb.tryPop(x)) {
    std::cout << x << std::endl;
  }
}
```

### Project Using the Library

- [async_logger](https://github.com/HuRuilizhen/async_logger): the library for asynchronous logging with mpsc ring buffer support

## Uninstall

```bash
sudo cmake --build build --target uninstall_ring_buffer
```

## Contributing

Contributions are welcome! Feel free to open issues or pull requests on GitHub.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

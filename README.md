# ring_buffer

![Build Status](https://img.shields.io/github/actions/workflow/status/HuRuilizhen/ring_buffer/cmake-multi-platform.yml?branch=release)

A header-only C++20 ring buffer library providing several synchronization
strategies. Unit tests and microbenchmarks are included. Designed for easy
integration via CMake's `find_package` or `FetchContent`.

## Features

- Basic, SPSC, MPSC, and MPMC variants
- Cache-friendly memory layout
- Atomics- and slot-based synchronization strategies
- CMake-friendly integration through `find_package` or `FetchContent`

## Requirements

- C++20-compatible compiler
- CMake >= 3.15
- (Optional) GoogleTest for unit tests
- (Optional) A CMake version recent enough to support presets for the
  recommended development workflow

## Table of Contents

- [ring\_buffer](#ring_buffer)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Table of Contents](#table-of-contents)
  - [Building](#building)
    - [Configuration](#configuration)
    - [Execution](#execution)
  - [Running Benchmarks](#running-benchmarks)
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
| `RING_BUFFER_BUILD_TESTS`       | OFF     | Build unit tests (requires GTest)         |
| `RING_BUFFER_BUILD_BENCHMARKS`  | OFF     | Build microbenchmarks                     |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | OFF     | Generate `compile_commands.json` for IDEs |

### Execution

```bash
# Recommended development build
cmake --preset debug
cmake --build --preset debug

# Release build
cmake --preset release
cmake --build --preset release

# Run tests
ctest --preset debug

# Build with tests + benchmarks explicitly enabled
cmake -S . -B build \
  -DRING_BUFFER_BUILD_TESTS=ON \
  -DRING_BUFFER_BUILD_BENCHMARKS=ON
cmake --build build

# Install from a configured build tree
sudo cmake --install build/release
```

## Running Benchmarks

```bash
./build/debug/bin/ring_buffer_benchmark
```

## Running Tests

```bash
ctest --preset debug
```

## Using the Library

### Including the Library

Without installation, pull `ring_buffer` via `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  ring_buffer
  GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
  GIT_TAG        v0.2.0
)

FetchContent_MakeAvailable(ring_buffer)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ring_buffer::ring_buffer)
```

or after installation:

```cmake
find_package(ring_buffer CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ring_buffer::ring_buffer)
```

### Supported APIs

| Class                | Model | Synchronization           |
| -------------------- | ----- | ------------------------- |
| `BasicRingBuffer<T>` | Basic | None (single-thread only) |
| `SPSCRingBuffer<T>`  | SPSC  | atomics                   |
| `MPSCRingBuffer<T>`  | MPSC  | atomics + slots           |
| `MPMCRingBuffer<T>`  | MPMC  | atomics + slots           |

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
#include <iostream>
#include <ring_buffer/basic_ring_buffer.h>

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

Contributions are welcome. Feel free to open issues or pull requests on GitHub.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

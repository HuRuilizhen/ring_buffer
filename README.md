# ring_buffer

![Build Status](https://img.shields.io/github/actions/workflow/status/HuRuilizhen/ring_buffer/cmake-multi-platform.yml?branch=release)

A header-only C++20 ring buffer library providing several synchronization strategies. Unit tests and microbenchmarks are included.

## Features

- Single-producer/single-consumer and multi-producer/multi-consumer variants
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
  - [Benchmarks](#benchmarks)
  - [Running Tests](#running-tests)
  - [Using the Library](#using-the-library)
    - [Using FetchContent](#using-fetchcontent)
  - [Uninstall](#uninstall)
  - [Contributing](#contributing)
  - [License](#license)

## Building

Choose your preferred CMake generator:

```bash
cmake -S . -B build -G Ninja    # or Xcode / "Unix Makefiles"
cmake --build build
```

Enable tests and benchmarks if desired:

```bash
cmake -S . -B build -DENABLE_TESTS=ON -DENABLE_BENCHMARKS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
sudo cmake --install build
```

Build without tests and benchmarks for a lean installation:

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
sudo cmake --install build
```

## Benchmarks

```bash
./build/bin/ring_buffer_benchmark
```

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Using the Library

After installation you can import the target in your own project:

```cmake
find_package(ring_buffer CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE ring_buffer::ring_buffer)
```

### Using FetchContent

You can also fetch the library automatically without installing it first:

```cmake
include(FetchContent)
FetchContent_Declare(
  ring_buffer
  GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer
  GIT_TAG        v1.0.1 # stable release on the release branch
)
FetchContent_MakeAvailable(ring_buffer)
target_link_libraries(your_app PRIVATE ring_buffer)
```

## Uninstall

```bash
sudo cmake --build build --target uninstall_ring_buffer
```

## Contributing

Contributions are welcome! Feel free to open issues or pull requests on GitHub.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.


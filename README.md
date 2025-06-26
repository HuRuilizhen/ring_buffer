# ring_buffer

A simple ring buffer implementation with GoogleTest-based unit tests
and basic benchmarks.

## Build Instructions

Choose your preferred build system.

```bash
cmake -G Ninja -B build
```

Or

```bash
cmake -G Xcode -B build
```

Or

```bash
cmake -G "Unix Makefiles" -B build
```

Then run the build and optionally install:

```bash
cmake -S . -B build
cmake --build build
sudo cmake --install build
```


### Running the Benchmark

After building, execute the benchmark program:

```bash
./build/bin/ring_buffer_benchmark
```

### Running Tests

Tests are built automatically. Execute them using CTest:

```bash
cd build
ctest --output-on-failure
```

## Using the Library

After installation you can use `find_package` to locate the
`ring_buffer` target in your own CMake project:

```cmake
find_package(ring_buffer CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE ring_buffer::ring_buffer)
```

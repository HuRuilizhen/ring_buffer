# ring_buffer

A simple ring buffer implementation with accompanying example and
GoogleTest-based unit tests.

## Build Instructions

```bash
cmake -S . -B build
cmake --build build
```

### Running the Example

After building, run the example program:

```bash
./build/bin/ring_buffer_example
```

### Running Tests

Tests are built automatically. Execute them using CTest:

```bash
cd build
ctest --output-on-failure
```

cmake_minimum_required(VERSION 3.10)

project(RingBufferProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(ring_buffer_example example/main.cc)

target_include_directories(ring_buffer_example PRIVATE src)

enable_testing()
find_package(GTest REQUIRED)

add_executable(ring_buffer_test tests/ring_buffer_test.cc)
target_include_directories(ring_buffer_test PRIVATE src)
target_link_libraries(ring_buffer_test GTest::gtest_main)

add_test(NAME ring_buffer_test COMMAND ring_buffer_test)

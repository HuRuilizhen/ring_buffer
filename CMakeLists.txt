cmake_minimum_required(VERSION 3.15)

project(
  ring_buffer
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(ring_buffer INTERFACE)
target_include_directories(
  ring_buffer INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                        $<INSTALL_INTERFACE:include>)

option(ENABLE_BENCHMARKS "Enable benchmarks" OFF)

if(ENABLE_BENCHMARKS)
  message(STATUS "Benchmakes enabled")

  add_executable(
    ring_buffer_benchmark
    benchmarks/benchmark_main.cc benchmarks/benchmark_atomic.cc
    benchmarks/benchmark_mutex.cc benchmarks/benchmark_semiatomic_slot.cc)
  target_link_libraries(ring_buffer_benchmark PRIVATE ring_buffer)
endif()

option(ENABLE_TESTS "Enable tests" OFF)

if(ENABLE_TESTS)
  message(STATUS "Tests enabled")

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()

  add_executable(ring_buffer_test tests/ring_buffer_test.cc)
  add_executable(ring_buffer_atomic_test tests/ring_buffer_atomic_test.cc)
  add_executable(ring_buffer_mutex_test tests/ring_buffer_mutex_test.cc)
  add_executable(ring_buffer_semiatomic_slot_test
                 tests/ring_buffer_semiatomic_slot_test.cc)
  target_link_libraries(ring_buffer_test PRIVATE ring_buffer GTest::gtest_main)
  target_link_libraries(ring_buffer_atomic_test PRIVATE ring_buffer
                                                        GTest::gtest_main)
  target_link_libraries(ring_buffer_mutex_test PRIVATE ring_buffer
                                                       GTest::gtest_main)
  target_link_libraries(ring_buffer_semiatomic_slot_test
                        PRIVATE ring_buffer GTest::gtest_main)
  add_test(NAME ring_buffer_test COMMAND ring_buffer_test)
  add_test(NAME ring_buffer_atomic_test COMMAND ring_buffer_atomic_test)
  add_test(NAME ring_buffer_mutex_test COMMAND ring_buffer_mutex_test)
  add_test(NAME ring_buffer_semiatomic_slot_test
           COMMAND ring_buffer_semiatomic_slot_test)
endif(ENABLE_TESTS)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ring_buffer EXPORT ring_bufferTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ring_bufferTargets
  FILE ring_bufferTargets.cmake
  NAMESPACE ring_buffer::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring_buffer)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ring_bufferConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ring_bufferConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ring_bufferConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring_buffer)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ring_bufferConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/ring_bufferConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring_buffer)

export(
  EXPORT ring_bufferTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/ring_bufferTargets.cmake
  NAMESPACE ring_buffer::)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Uninstall.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P
                                    ${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake)

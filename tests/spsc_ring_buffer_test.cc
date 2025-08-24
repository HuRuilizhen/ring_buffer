#include "ring_buffer/spsc_ring_buffer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <thread>
#include <vector>

namespace {

struct Counter {
  static inline int constructed = 0;
  static inline int destructed = 0;
  static inline int moved = 0;
  static inline int copied = 0;

  int value;

  Counter(int v = 0) : value(v) { constructed++; }
  Counter(const Counter& other) : value(other.value) {
    copied++;
    constructed++;
  }
  Counter(Counter&& other) noexcept : value(other.value) {
    moved++;
    constructed++;
  }
  ~Counter() { destructed++; }

  static void reset() { constructed = destructed = moved = copied = 0; }
};

}  // namespace

TEST(SPSCRingBufferTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::SPSCRingBuffer<int> buffer(0),
               std::invalid_argument);
}

TEST(SPSCRingBufferTest, PushPopBasic) {
  int value;
  RingBuffer::SPSCRingBuffer<int> buffer(3);

  buffer.tryPush(1);
  buffer.tryPush(2);
  buffer.tryPush(3);
  EXPECT_FALSE(buffer.tryPush(-1));

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 1);

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 2);

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 3);
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(SPSCRingBufferTest, WrapAround) {
  int value;
  RingBuffer::SPSCRingBuffer<int> buffer(3);

  buffer.tryPush(1);
  buffer.tryPush(2);
  buffer.tryPush(3);
  EXPECT_FALSE(buffer.tryPush(-1));

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 1);
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 2);
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 3);

  EXPECT_TRUE(buffer.tryPush(4));
  EXPECT_TRUE(buffer.tryPush(5));
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 4);
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 5);
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(SPSCRingBufferTest, SPSC) {
  constexpr int ITERATIONS = 10000;
  constexpr int CAPACITY = 128;
  RingBuffer::SPSCRingBuffer<int> buffer(CAPACITY);

  std::thread producer([&]() {
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        if (buffer.tryPush(i)) break;
        std::this_thread::yield();
      }
    }
  });

  std::vector<int> results;
  results.reserve(ITERATIONS);
  std::thread consumer([&]() {
    int value;
    for (int i = 0; i < ITERATIONS; ++i) {
      while (true) {
        if (buffer.tryPop(value)) break;
        std::this_thread::yield();
      }
      results.push_back(value);
    }
  });

  producer.join();
  consumer.join();

  for (int i = 0; i < ITERATIONS; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

TEST(SPSCRingBufferTest, EmplaceBasicType) {
  int value;
  RingBuffer::SPSCRingBuffer<int> buffer(2);

  EXPECT_TRUE(buffer.tryEmplace(42));
  EXPECT_TRUE(buffer.tryEmplace(99));
  EXPECT_FALSE(buffer.tryEmplace(123));
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 42);
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 99);
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(SPSCRingBufferTest, EmplaceString) {
  std::string value;
  RingBuffer::SPSCRingBuffer<std::string> buffer(2);

  EXPECT_TRUE(buffer.tryEmplace("hello"));
  EXPECT_TRUE(buffer.tryEmplace(5, 'x'));
  EXPECT_FALSE(buffer.tryEmplace("world"));
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, "hello");
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, "xxxxx");
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(RingBufferTest, CounterLifecycle) {
  Counter::reset();
  {
    RingBuffer::SPSCRingBuffer<Counter> buffer(2);

    EXPECT_TRUE(buffer.tryEmplace(1));
    EXPECT_TRUE(buffer.tryEmplace(2));
    EXPECT_FALSE(buffer.tryEmplace(3));

    Counter tmp;
    EXPECT_TRUE(buffer.tryPop(tmp));
    EXPECT_TRUE(buffer.tryPop(tmp));
    EXPECT_FALSE(buffer.tryPop(tmp));
  }

  EXPECT_EQ(Counter::constructed, Counter::destructed);
}

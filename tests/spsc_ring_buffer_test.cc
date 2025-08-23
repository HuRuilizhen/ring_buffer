#include "ring_buffer/spsc_ring_buffer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <thread>
#include <vector>

TEST(RingBufferAtomicTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::SPSCRingBuffer<int> buffer(0),
               std::invalid_argument);
}

TEST(RingBufferAtomicTest, PushPopBasic) {
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

TEST(RingBufferAtomicTest, WrapAround) {
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

TEST(RingBufferAtomicTest, SPSC) {
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

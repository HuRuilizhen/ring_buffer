#include "ring_buffer/ring_buffer_atomic.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <thread>
#include <vector>

TEST(RingBufferAtomicTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::RingBufferAtomic<int> buffer(0),
               std::invalid_argument);
}

TEST(RingBufferAtomicTest, PushPopBasic) {
  RingBuffer::RingBufferAtomic<int> buffer(3);
  EXPECT_TRUE(buffer.isEmpty());
  EXPECT_FALSE(buffer.isFull());
  EXPECT_EQ(buffer.getCapacity(), 3u);
  EXPECT_EQ(buffer.getSize(), 0u);

  buffer.push(1);
  buffer.push(2);
  EXPECT_EQ(buffer.getSize(), 2u);
  EXPECT_FALSE(buffer.isFull());
  EXPECT_FALSE(buffer.isEmpty());

  buffer.push(3);
  EXPECT_TRUE(buffer.isFull());
  EXPECT_EQ(buffer.getSize(), 3u);
  EXPECT_THROW(buffer.push(4), std::overflow_error);

  EXPECT_EQ(buffer.pop(), 1);
  EXPECT_EQ(buffer.pop(), 2);
  EXPECT_FALSE(buffer.isFull());
  EXPECT_FALSE(buffer.isEmpty());
  EXPECT_EQ(buffer.pop(), 3);
  EXPECT_TRUE(buffer.isEmpty());
  EXPECT_THROW(buffer.pop(), std::underflow_error);
}

TEST(RingBufferAtomicTest, WrapAround) {
  RingBuffer::RingBufferAtomic<int> buffer(3);
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);
  EXPECT_TRUE(buffer.isFull());

  EXPECT_EQ(buffer.pop(), 1);
  EXPECT_EQ(buffer.pop(), 2);
  EXPECT_FALSE(buffer.isFull());

  buffer.push(4);
  buffer.push(5);
  EXPECT_TRUE(buffer.isFull());

  EXPECT_EQ(buffer.pop(), 3);
  EXPECT_EQ(buffer.pop(), 4);
  EXPECT_EQ(buffer.pop(), 5);
}

TEST(RingBufferAtomicTest, SPSC) {
  constexpr int iterations = 10000;
  RingBuffer::RingBufferAtomic<int> buffer(128);

  std::thread producer([&]() {
    for (int i = 0; i < iterations; ++i) {
      while (true) {
        try {
          buffer.push(i);
          break;
        } catch (const std::overflow_error &) {
          std::this_thread::yield();
        }
      }
    }
  });

  std::vector<int> results;
  results.reserve(iterations);
  std::thread consumer([&]() {
    for (int i = 0; i < iterations; ++i) {
      int value;
      while (true) {
        try {
          value = buffer.pop();
          break;
        } catch (const std::underflow_error &) {
          std::this_thread::yield();
        }
      }
      results.push_back(value);
    }
  });

  producer.join();
  consumer.join();

  for (int i = 0; i < iterations; ++i) {
    EXPECT_EQ(results[i], i);
  }
  EXPECT_TRUE(buffer.isEmpty());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

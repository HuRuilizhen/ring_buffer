#include "ring_buffer.h"

#include <gtest/gtest.h>

TEST(RingBufferTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::RingBuffer<int> buffer(0), std::invalid_argument);
}

TEST(RingBufferTest, PushPopBasic) {
  RingBuffer::RingBuffer<int> buffer(3);
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

TEST(RingBufferTest, WrapAround) {
  RingBuffer::RingBuffer<int> buffer(3);
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

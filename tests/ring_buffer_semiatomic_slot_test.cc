#include <gtest/gtest.h>

#include "ring_buffer/internal/semiatomic_slot.h"

TEST(RingBufferTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::RingBufferSemiAtomicSlot<int> buffer(0),
               std::invalid_argument);
}

TEST(RingBufferTest, PushPopBasic) {
  int value;
  RingBuffer::RingBufferSemiAtomicSlot<int> buffer(3);
  EXPECT_TRUE(buffer.isEmpty());
  EXPECT_EQ(buffer.getCapacity(), 3u);

  buffer.tryPush(1);
  buffer.tryPush(2);
  buffer.tryPush(3);
  EXPECT_FALSE(buffer.tryPush(-1));

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 1);

  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 2);

  EXPECT_FALSE(buffer.isEmpty());
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 3);
  EXPECT_TRUE(buffer.isEmpty());
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(RingBufferTest, WrapAround) {
  int value;
  RingBuffer::RingBufferSemiAtomicSlot<int> buffer(3);

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

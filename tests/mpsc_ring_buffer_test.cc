#include "ring_buffer/mpsc_ring_buffer.h"

#include <gtest/gtest.h>

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

TEST(MPSCRingBufferTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::MPSCRingBuffer<int> buffer(0),
               std::invalid_argument);
}

TEST(MPSCRingBufferTest, PushPopBasic) {
  int value;
  RingBuffer::MPSCRingBuffer<int> buffer(3);

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

TEST(MPSCRingBufferTest, WrapAround) {
  int value;
  RingBuffer::MPSCRingBuffer<int> buffer(3);

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

TEST(MPSCRingBufferTest, EmplaceBasicType) {
  int value;
  RingBuffer::MPSCRingBuffer<int> buffer(2);

  EXPECT_TRUE(buffer.tryEmplace(42));
  EXPECT_TRUE(buffer.tryEmplace(99));
  EXPECT_FALSE(buffer.tryEmplace(123));
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 42);
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, 99);
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(MPSCRingBufferTest, EmplaceString) {
  std::string value;
  RingBuffer::MPSCRingBuffer<std::string> buffer(2);

  EXPECT_TRUE(buffer.tryEmplace("hello"));
  EXPECT_TRUE(buffer.tryEmplace(5, 'x'));
  EXPECT_FALSE(buffer.tryEmplace("world"));
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, "hello");
  EXPECT_TRUE(buffer.tryPop(value));
  EXPECT_EQ(value, "xxxxx");
  EXPECT_FALSE(buffer.tryPop(value));
}

TEST(MPSCRingBufferTest, CounterLifecycle) {
  const int CAPACITY = 2;
  Counter::reset();

  {
    RingBuffer::MPSCRingBuffer<Counter> buffer(CAPACITY);

    EXPECT_TRUE(buffer.tryEmplace(1));
    EXPECT_TRUE(buffer.tryEmplace(2));
    EXPECT_FALSE(buffer.tryEmplace(3));

    Counter tmp;
    EXPECT_TRUE(buffer.tryPop(tmp));
    EXPECT_TRUE(buffer.tryPop(tmp));
    EXPECT_FALSE(buffer.tryPop(tmp));
  }

  // the init phase should be cleaned up
  EXPECT_EQ(Counter::constructed - CAPACITY, Counter::destructed);
}

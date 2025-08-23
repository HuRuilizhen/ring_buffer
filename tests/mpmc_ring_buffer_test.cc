#include "ring_buffer/mpmc_ring_buffer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

TEST(RingBufferMutexTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::MPMCRingBuffer<int> buffer(0),
               std::invalid_argument);
}

TEST(RingBufferMutexTest, PushPopBasic) {
  int value;
  RingBuffer::MPMCRingBuffer<int> buffer(3);

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

TEST(RingBufferMutexTest, WrapAround) {
  int value;
  RingBuffer::MPMCRingBuffer<int> buffer(3);

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

TEST(RingBufferMutexTest, SPSC) {
  constexpr int ITERATIONS = 10000;
  constexpr int CAPACITY = 128;
  RingBuffer::MPMCRingBuffer<int> buffer(CAPACITY);

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

TEST(RingBufferMutexTest, MPMC) {
  constexpr int PRODUCERS = 4;
  constexpr int ITEMS_PER_PRODUCER = 250;
  constexpr int TOTAL_ITEMS = PRODUCERS * ITEMS_PER_PRODUCER;
  constexpr int CAPACITY = 128;

  RingBuffer::MPMCRingBuffer<int> buffer(CAPACITY);
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};
  std::vector<std::thread> producer_threads;
  for (int p = 0; p < PRODUCERS; ++p) {
    producer_threads.emplace_back([p, &buffer, &produced]() {
      for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int value = p * ITEMS_PER_PRODUCER + i;
        while (true) {
          if (buffer.tryPush(value)) {
            produced.fetch_add(1, std::memory_order_relaxed);
            break;
          } else
            std::this_thread::yield();
        }
      }
    });
  }

  std::vector<int> results;
  results.reserve(TOTAL_ITEMS);
  std::mutex results_mtx;
  std::vector<std::thread> consumer_threads;
  for (int c = 0; c < PRODUCERS; ++c) {
    consumer_threads.emplace_back([&]() {
      while (true) {
        int value;
        if (buffer.tryPop(value)) {
          {
            std::lock_guard<std::mutex> lock(results_mtx);
            results.push_back(value);
          }
          if (consumed.fetch_add(1, std::memory_order_relaxed) + 1 >=
              TOTAL_ITEMS)
            break;
        } else {
          if (produced.load(std::memory_order_acquire) >= TOTAL_ITEMS) {
            if (consumed.load(std::memory_order_relaxed) >= TOTAL_ITEMS) break;
          }
          std::this_thread::yield();
        }
      }
    });
  }

  for (auto& t : producer_threads) t.join();
  for (auto& t : consumer_threads) t.join();

  EXPECT_EQ(results.size(), static_cast<size_t>(TOTAL_ITEMS));
  std::sort(results.begin(), results.end());
  for (int i = 0; i < TOTAL_ITEMS; ++i) {
    EXPECT_EQ(results[i], i);
  }
}

#include "ring_buffer/ring_buffer_mutex.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

TEST(RingBufferMutexTest, ConstructorInvalid) {
  EXPECT_THROW(RingBuffer::RingBufferMutex<int> buffer(0),
               std::invalid_argument);
}

TEST(RingBufferMutexTest, PushPopBasic) {
  RingBuffer::RingBufferMutex<int> buffer(3);
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

TEST(RingBufferMutexTest, WrapAround) {
  RingBuffer::RingBufferMutex<int> buffer(3);
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

TEST(RingBufferMutexTest, SPSC) {
  constexpr int iterations = 10000;
  RingBuffer::RingBufferMutex<int> buffer(128);

  std::thread producer([&]() {
    for (int i = 0; i < iterations; ++i) {
      while (true) {
        try {
          buffer.push(i);
          break;
        } catch (const std::overflow_error&) {
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
        } catch (const std::underflow_error&) {
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

TEST(RingBufferMutexTest, MPMC) {
  constexpr int producers = 4;
  constexpr int items_per_producer = 250;
  constexpr int total_items = producers * items_per_producer;

  RingBuffer::RingBufferMutex<int> buffer(128);
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};
  std::vector<std::thread> producer_threads;
  for (int p = 0; p < producers; ++p) {
    producer_threads.emplace_back([p, &buffer, &produced]() {
      for (int i = 0; i < items_per_producer; ++i) {
        int value = p * items_per_producer + i;
        while (true) {
          try {
            buffer.push(value);
            produced.fetch_add(1, std::memory_order_relaxed);
            break;
          } catch (const std::overflow_error&) {
            std::this_thread::yield();
          }
        }
      }
    });
  }

  std::vector<int> results;
  results.reserve(total_items);
  std::mutex results_mtx;
  std::vector<std::thread> consumer_threads;
  for (int c = 0; c < producers; ++c) {
    consumer_threads.emplace_back([&]() {
      while (true) {
        int value;
        try {
          value = buffer.pop();
          {
            std::lock_guard<std::mutex> lock(results_mtx);
            results.push_back(value);
          }
          if (consumed.fetch_add(1, std::memory_order_relaxed) + 1 >=
              total_items)
            break;
        } catch (const std::underflow_error&) {
          if (produced.load(std::memory_order_acquire) >= total_items) {
            if (consumed.load(std::memory_order_relaxed) >= total_items) break;
          }
          std::this_thread::yield();
        }
      }
    });
  }

  for (auto& t : producer_threads) t.join();
  for (auto& t : consumer_threads) t.join();

  EXPECT_EQ(results.size(), static_cast<size_t>(total_items));
  std::sort(results.begin(), results.end());
  for (int i = 0; i < total_items; ++i) {
    EXPECT_EQ(results[i], i);
  }
  EXPECT_TRUE(buffer.isEmpty());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

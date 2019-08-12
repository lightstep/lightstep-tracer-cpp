#include "common/circular_buffer.h"

#include <random>
#include <thread>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

static thread_local std::mt19937 RandomNumberGenerator{std::random_device{}()};

static void GenerateRandomNumbers(CircularBuffer<uint32_t>& buffer,
                                  std::vector<uint32_t>& numbers, int n) {
  for (int i = 0; i < n; ++i) {
    auto value = static_cast<uint32_t>(RandomNumberGenerator());
    std::unique_ptr<uint32_t> x{new uint32_t{value}};
    if (buffer.Add(x)) {
      numbers.push_back(value);
    }
  }
}

static void RunNumberProducers(CircularBuffer<uint32_t>& buffer,
                               std::vector<uint32_t>& numbers, int num_threads,
                               int n) {
  std::vector<std::vector<uint32_t>> thread_numbers(num_threads);
  std::vector<std::thread> threads(num_threads);
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads[thread_index] =
        std::thread{GenerateRandomNumbers, std::ref(buffer),
                    std::ref(thread_numbers[thread_index]), n};
  }
  for (auto& thread : threads) {
    thread.join();
  }
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    numbers.insert(numbers.end(), thread_numbers[thread_index].begin(),
                   thread_numbers[thread_index].end());
  }
}

void RunNumberConsumer(CircularBuffer<uint32_t>& buffer,
                       std::atomic<bool>& exit,
                       std::vector<uint32_t>& numbers) {
  while (true) {
    auto allotment = buffer.Peek();
    if (exit && allotment.empty()) {
      return;
    }
    auto n = std::uniform_int_distribution<size_t>{
        0, allotment.size()}(RandomNumberGenerator);
    buffer.Consume(
        n, [&](CircularBufferRange<AtomicUniquePtr<uint32_t>> range) noexcept {
          REQUIRE(range.size() == n);
          range.ForEach([&](AtomicUniquePtr<uint32_t> & ptr) noexcept {
            REQUIRE(!ptr.IsNull());
            numbers.push_back(*ptr);
            ptr.Reset();
            return true;
          });
        });
  }
}

TEST_CASE("CircularBuffer") {
  CircularBuffer<int> buffer{10};

  SECTION("We can add values into a circular buffer") {
    std::unique_ptr<int> x{new int{11}};
    REQUIRE(buffer.Add(x));
    REQUIRE(x == nullptr);
    auto range = buffer.Peek();
    REQUIRE(range.size() == 1);
    range.ForEach([](const AtomicUniquePtr<int>& y) {
      REQUIRE(*y == 11);
      return true;
    });
  }

  SECTION("We can clear a circular buffer") {
    std::unique_ptr<int> x{new int{11}};
    REQUIRE(buffer.Add(x));
    REQUIRE(x == nullptr);
    buffer.Clear();
    REQUIRE(buffer.empty());
  }

  SECTION("If CircularBuffer is full, Add fails") {
    for (int i = 0; i < static_cast<int>(buffer.max_size()); ++i) {
      std::unique_ptr<int> x{new int{i}};
      REQUIRE(buffer.Add(x));
    }
    std::unique_ptr<int> x{new int{33}};
    REQUIRE(!buffer.Add(x));
    REQUIRE(x != nullptr);
    REQUIRE(*x == 33);
  }

  SECTION("We can consume elements out of CircularBuffer") {
    for (int i = 0; i < static_cast<int>(buffer.max_size()); ++i) {
      std::unique_ptr<int> x{new int{i}};
      REQUIRE(buffer.Add(x));
    }
    int count = 0;
    buffer.Consume(
        5, [&](CircularBufferRange<AtomicUniquePtr<int>> range) noexcept {
          range.ForEach([&](AtomicUniquePtr<int>& ptr) {
            REQUIRE(*ptr == count++);
            ptr.Reset();
            return true;
          });
        });
    REQUIRE(count == 5);
  }
}

TEST_CASE("Verify CircularBuffer behaves correctly through simulation") {
  const int num_producer_threads = 4;
  const int n = 25000;
  for (size_t max_size : {1, 2, 10, 50, 100, 1000}) {
    CircularBuffer<uint32_t> buffer{max_size};
    std::vector<uint32_t> producer_numbers;
    std::vector<uint32_t> consumer_numbers;
    auto producers =
        std::thread{RunNumberProducers, std::ref(buffer),
                    std::ref(producer_numbers), num_producer_threads, n};
    std::atomic<bool> exit{false};
    auto consumer = std::thread{RunNumberConsumer, std::ref(buffer),
                                std::ref(exit), std::ref(consumer_numbers)};
    producers.join();
    exit = true;
    consumer.join();
    std::sort(producer_numbers.begin(), producer_numbers.end());
    std::sort(consumer_numbers.begin(), consumer_numbers.end());
    REQUIRE(producer_numbers == consumer_numbers);
  }
}

#include "common/chunk_circular_buffer.h"
#include "common/bipart_memory_stream.h"
#include "common/utility.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <opentracing/string_view.h>

#include <array>
#include <atomic>
#include <cstring>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

static std::string ToString(const CircularBufferConstPlacement& placement) {
  std::string s;
  s.append(placement.data1, placement.size1);
  s.append(placement.data2, placement.size2);
  return s;
}

static void WriteString(google::protobuf::io::CodedOutputStream& stream,
                        size_t size, void* data) {
  stream.WriteRaw(data, static_cast<int>(size));
}

static bool AddString(ChunkCircularBuffer& buffer, opentracing::string_view s) {
  return buffer.Add(WriteString, s.size(),
                    static_cast<void*>(const_cast<char*>(s.data())));
}

static thread_local std::mt19937 RandomNumberGenerator{std::random_device{}()};

static std::tuple<uint32_t, opentracing::string_view> GenerateRandomNumber(
    size_t max_digits) {
  assert(max_digits <= 32);
  static thread_local std::array<char, 32> number_buffer;
  std::uniform_int_distribution<int> num_digits_distribution{
      1, static_cast<int>(max_digits)};
  std::bernoulli_distribution digit_distribution{0.5};
  auto num_digits = num_digits_distribution(RandomNumberGenerator);
  uint32_t value = 0;
  for (int i = 0; i < num_digits; ++i) {
    value *= 2;
    if (digit_distribution(RandomNumberGenerator)) {
      number_buffer[i] = '1';
      value += 1;
    } else {
      number_buffer[i] = '0';
    }
  }
  return {value, {number_buffer.data(), static_cast<size_t>(num_digits)}};
}

static uint32_t ReadNumber(google::protobuf::io::ZeroCopyInputStream& stream,
                           size_t num_digits) {
  const char* data;
  int size;
  uint32_t result = 0;
  size_t digit_index = 0;
  while (stream.Next(reinterpret_cast<const void**>(&data), &size)) {
    for (int i = 0; i < size; ++i) {
      if (digit_index++ == num_digits) {
        stream.BackUp(size - i);
        return result;
      }
      result = 2 * result + static_cast<uint32_t>(data[i] == '1');
    }
  }
  if (digit_index != num_digits) {
    std::cerr << "unexpected number of digits\n";
    std::terminate();
  }
  return result;
}

static void GenerateRandomNumbers(ChunkCircularBuffer& buffer,
                                  std::vector<uint32_t>& numbers, size_t n) {
  assert(buffer.max_size() > 5);
  size_t max_digits = std::min<size_t>(buffer.max_size() - 5, 32);
  while (n-- != 0) {
    uint32_t x;
    opentracing::string_view s;
    std::tie(x, s) = GenerateRandomNumber(max_digits);
    if (AddString(buffer, s)) {
      numbers.push_back(x);
    }
  }
}

static void RunProducer(ChunkCircularBuffer& buffer,
                        std::vector<uint32_t>& numbers, size_t num_threads,
                        size_t n) {
  std::vector<std::vector<uint32_t>> thread_numbers(num_threads);
  std::vector<std::thread> threads(num_threads);
  for (size_t thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads[thread_index] =
        std::thread{GenerateRandomNumbers, std::ref(buffer),
                    std::ref(thread_numbers[thread_index]), n};
  }
  for (auto& thread : threads) {
    thread.join();
  }
  for (size_t thread_index = 0; thread_index < num_threads; ++thread_index) {
    numbers.insert(numbers.end(), thread_numbers[thread_index].begin(),
                   thread_numbers[thread_index].end());
  }
}

static void RunConsumer(ChunkCircularBuffer& buffer, std::atomic<bool>& exit,
                        std::vector<uint32_t>& numbers) {
  while (!exit || !buffer.empty()) {
    buffer.Allot();
    auto allotment = buffer.allotment();
    BipartMemoryInputStream stream{allotment.data1, allotment.size1,
                                   allotment.data2, allotment.size2};
    size_t num_digits;
    while (ReadChunkHeader(stream, num_digits)) {
      assert(num_digits != 0);
      auto x = ReadNumber(stream, num_digits);
      numbers.push_back(x);
      assert(stream.Skip(2));
    }
    buffer.Consume(buffer.num_bytes_allotted());
  }
}

TEST_CASE(
    "Verify through simulation that ChunkCircularBuffer behaves correctly.") {
  const size_t num_producer_threads = 4;
  const size_t n = 25000;
  for (size_t max_size : {10, 50, 100, 1000}) {
    ChunkCircularBuffer buffer{max_size};
    std::vector<uint32_t> producer_numbers;
    std::vector<uint32_t> consumer_numbers;
    auto producer =
        std::thread{RunProducer, std::ref(buffer), std::ref(producer_numbers),
                    num_producer_threads, n};
    std::atomic<bool> exit{false};
    auto consumer = std::thread{RunConsumer, std::ref(buffer), std::ref(exit),
                                std::ref(consumer_numbers)};
    producer.join();
    exit = true;
    consumer.join();
    std::sort(producer_numbers.begin(), producer_numbers.end());
    std::sort(consumer_numbers.begin(), consumer_numbers.end());
    REQUIRE(producer_numbers == consumer_numbers);
  }
}

TEST_CASE(
    "ChunkCircularBuffer stores serialized messages surrounded by http/1.1 "
    "chunk framing.") {
  ChunkCircularBuffer buffer{13};
  REQUIRE(buffer.num_bytes_allotted() == 0);
  REQUIRE(buffer.allotment().size1 == 0);

  SECTION("Allot does nothing if the buffer is empty.") {
    buffer.Allot();
    REQUIRE(buffer.num_bytes_allotted() == 0);
    REQUIRE(buffer.allotment().size1 == 0);
  }

  SECTION(
      "Messages added to the buffer are surrounded by http/1.1 chunk "
      "framing.") {
    REQUIRE(AddString(buffer, "abc"));
    buffer.Allot();
    REQUIRE(ToString(buffer.allotment()) == "3\r\nabc\r\n");
  }

  SECTION("Adding a message fails if it would exceed the size of the buffer.") {
    REQUIRE(AddString(buffer, "abc"));
    REQUIRE(!AddString(buffer, "abc"));
  }

  SECTION("Successive messages get serialized one after the other together.") {
    REQUIRE(AddString(buffer, "ab"));
    REQUIRE(AddString(buffer, "c"));
    buffer.Allot();
    REQUIRE(buffer.num_bytes_allotted() == 13);
    REQUIRE(ToString(buffer.allotment()) == "2\r\nab\r\n1\r\nc\r\n");
  }

  SECTION("Allot picks up where the last allotment left.") {
    REQUIRE(AddString(buffer, "ab"));
    buffer.Allot();
    REQUIRE(AddString(buffer, "c"));
    buffer.Allot();
    REQUIRE(buffer.num_bytes_allotted() == 13);
    REQUIRE(ToString(buffer.allotment()) == "2\r\nab\r\n1\r\nc\r\n");
  }

  SECTION("Once space has been allotted, it can be consumed.") {
    REQUIRE(AddString(buffer, "ab"));
    REQUIRE(AddString(buffer, "c"));
    buffer.Allot();
    buffer.Consume(7);
    REQUIRE(buffer.num_bytes_allotted() == 6);
    REQUIRE(ToString(buffer.allotment()) == "1\r\nc\r\n");
  }
}

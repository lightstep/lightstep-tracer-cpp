#include "test/number_simulation.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <thread>

#include "common/bipart_memory_stream.h"
#include "common/utility.h"
#include "test/utility.h"

static thread_local std::mt19937 RandomNumberGenerator{std::random_device{}()};

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GenerateRandomBinaryNumbers
//--------------------------------------------------------------------------------------------------
static void GenerateRandomBinaryNumbers(ChunkCircularBuffer& buffer,
                                        std::vector<uint32_t>& numbers,
                                        size_t n) {
  assert(buffer.max_size() > 5);
  size_t max_digits = std::min<size_t>(buffer.max_size() - 5, 32);
  while (n-- != 0) {
    uint32_t x;
    opentracing::string_view s;
    std::tie(x, s) = GenerateRandomBinaryNumber(max_digits);
    if (AddString(buffer, s)) {
      numbers.push_back(x);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// GenerateRandomBinaryNumber
//--------------------------------------------------------------------------------------------------
std::tuple<uint32_t, opentracing::string_view> GenerateRandomBinaryNumber(
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

//--------------------------------------------------------------------------------------------------
// ReadBinaryNumber
//--------------------------------------------------------------------------------------------------
uint32_t ReadBinaryNumber(google::protobuf::io::ZeroCopyInputStream& stream,
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

//--------------------------------------------------------------------------------------------------
// RunBinaryNumberProducer
//--------------------------------------------------------------------------------------------------
void RunBinaryNumberProducer(ChunkCircularBuffer& buffer,
                             std::vector<uint32_t>& numbers, size_t num_threads,
                             size_t n) {
  std::vector<std::vector<uint32_t>> thread_numbers(num_threads);
  std::vector<std::thread> threads(num_threads);
  for (size_t thread_index = 0; thread_index < num_threads; ++thread_index) {
    threads[thread_index] =
        std::thread{GenerateRandomBinaryNumbers, std::ref(buffer),
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

//--------------------------------------------------------------------------------------------------
// RunBinaryNumberConsumer
//--------------------------------------------------------------------------------------------------
void RunBinaryNumberConsumer(ChunkCircularBuffer& buffer,
                             std::atomic<bool>& exit,
                             std::vector<uint32_t>& numbers) {
  while (!exit || !buffer.empty()) {
    buffer.Allot();
    auto allotment = buffer.allotment();
    BipartMemoryInputStream stream{allotment.data1, allotment.size1,
                                   allotment.data2, allotment.size2};
    size_t num_digits;
    while (ReadChunkHeader(stream, num_digits)) {
      assert(num_digits != 0);
      auto x = ReadBinaryNumber(stream, num_digits);
      numbers.push_back(x);
      assert(stream.Skip(2));
    }
    buffer.Consume(buffer.num_bytes_allotted());
  }
}
}  // namespace lightstep

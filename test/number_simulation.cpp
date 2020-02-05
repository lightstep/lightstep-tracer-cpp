#include "test/number_simulation.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include "common/utility.h"
#include "test/utility.h"
#include "test/zero_copy_connection_input_stream.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

static thread_local std::mt19937 RandomNumberGenerator{std::random_device{}()};

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GenerateRandomBinaryNumber
//--------------------------------------------------------------------------------------------------
static std::tuple<uint32_t, opentracing::string_view>
GenerateRandomBinaryNumber(size_t max_digits) {
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
// GenerateRandomBinaryNumbers
//--------------------------------------------------------------------------------------------------
static void GenerateRandomBinaryNumbers(CircularBuffer<ChainedStream>& buffer,
                                        std::vector<uint32_t>& numbers,
                                        size_t n) {
  while (n-- != 0) {
    uint32_t x;
    opentracing::string_view s;
    std::tie(x, s) = GenerateRandomBinaryNumber(32);
    if (AddSpanChunkFramedString(buffer, s)) {
      numbers.push_back(x);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// ReadStreamHeader
//--------------------------------------------------------------------------------------------------
static void ReadStreamHeader(
    google::protobuf::io::ZeroCopyInputStream& stream) {
  size_t chunk_size;
  if (!ReadChunkHeader(stream, chunk_size)) {
    std::cerr << "ReadChunkHeader failed\n";
    std::terminate();
  }
  {
    google::protobuf::io::LimitingInputStream limit_stream{
        &stream, static_cast<int>(chunk_size)};
    collector::ReportRequest report;
    if (!report.ParseFromZeroCopyStream(&limit_stream)) {
      std::cerr << "Failed to parse stream header\n";
      std::terminate();
    }
  }
  stream.Skip(2);
}

//--------------------------------------------------------------------------------------------------
// ReadBinaryNumber
//--------------------------------------------------------------------------------------------------
static uint32_t ReadBinaryNumber(
    google::protobuf::io::ZeroCopyInputStream& stream, size_t num_digits) {
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
// ReadBinaryNumberChunk
//--------------------------------------------------------------------------------------------------
static bool ReadBinaryNumberChunk(
    google::protobuf::io::ZeroCopyInputStream& stream, uint32_t& x) {
  size_t chunk_size;
  if (!ReadChunkHeader(stream, chunk_size)) {
    return false;
  }

  if (chunk_size == 0) {
    // We reached the terminal chunk
    stream.Skip(2);
    return false;
  }

  size_t num_digits = [&] {
    google::protobuf::io::CodedInputStream coded_stream{&stream};
    google::protobuf::uint32 field_number;
    if (!coded_stream.ReadVarint32(&field_number)) {
      std::cerr << "ReadVarint32 failed\n";
      std::terminate();
    }
    google::protobuf::uint64 num_digits;
    if (!coded_stream.ReadVarint64(&num_digits)) {
      std::cerr << "ReadVarint64 failed\n";
      std::terminate();
    }
    return static_cast<size_t>(num_digits);
  }();

  x = ReadBinaryNumber(stream, num_digits);
  stream.Skip(2);
  return true;
}

//--------------------------------------------------------------------------------------------------
// HasPendingData
//--------------------------------------------------------------------------------------------------
static bool HasPendingData(ConnectionStream& connection_stream) {
  bool result = false;
  connection_stream.Flush(
      [&result](std::initializer_list<FragmentInputStream*> streams) {
        for (auto stream : streams) {
          if (stream->num_fragments() > 0) {
            result = true;
            return false;
          }
        }
        return true;
      });
  return result;
}

//--------------------------------------------------------------------------------------------------
// RunBinaryNumberProducer
//--------------------------------------------------------------------------------------------------
void RunBinaryNumberProducer(CircularBuffer<ChainedStream>& buffer,
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
// RunBinaryNumberConnectionConsumer
//--------------------------------------------------------------------------------------------------
void RunBinaryNumberConnectionConsumer(
    SpanStream& span_stream, std::vector<ConnectionStream>& connection_streams,
    std::atomic<bool>& exit, std::vector<uint32_t>& numbers) {
  std::vector<std::unique_ptr<ZeroCopyConnectionInputStream>> zero_copy_streams;
  zero_copy_streams.reserve(connection_streams.size());
  for (auto& connection_stream : connection_streams) {
    zero_copy_streams.emplace_back(
        new ZeroCopyConnectionInputStream{connection_stream});
    zero_copy_streams.back()->Skip(connection_stream.first_chunk_position());
    ReadStreamHeader(*zero_copy_streams.back());
  }

  std::cout << "reading numbers" << std::endl;
  std::uniform_int_distribution<int> distribution{
      0, static_cast<int>(connection_streams.size()) - 1};
  while (true) {
    if (exit) {
      span_stream.Allot();
      if (span_stream.empty()) {
        break;
      }
    }
    auto connection_index = distribution(RandomNumberGenerator);
    auto& connection_stream = connection_streams[connection_index];
    if (!HasPendingData(connection_stream)) {
      continue;
    }
    auto& stream = *zero_copy_streams[connection_index];
    uint32_t x;
    if (!ReadBinaryNumberChunk(stream, x)) {
      std::cerr << "ReadBinaryNumberChunk failed\n";
      std::terminate();
    }
    numbers.push_back(x);
  }
  std::cout << "shutting down" << std::endl;
  for (auto& connection_stream : connection_streams) {
    connection_stream.Shutdown();
  }
  for (auto& stream : zero_copy_streams) {
    uint32_t x;
    while (ReadBinaryNumberChunk(*stream, x)) {
      numbers.push_back(x);
    }
  }
}
}  // namespace lightstep

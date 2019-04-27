#include "common/chunk_circular_buffer.h"
#include "common/utility.h"
#include "test/utility.h"

#include <opentracing/string_view.h>

#include <array>
#include <atomic>
#include <cstring>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

#include "3rd_party/catch2/catch.hpp"
#include "test/number_simulation.h"
using namespace lightstep;

TEST_CASE(
    "Verify through simulation that ChunkCircularBuffer behaves correctly.") {
  const size_t num_producer_threads = 4;
  const size_t n = 25000;
  for (size_t max_size : {10, 50, 100, 1000}) {
    ChunkCircularBuffer buffer{max_size};
    std::vector<uint32_t> producer_numbers;
    std::vector<uint32_t> consumer_numbers;
    auto producer =
        std::thread{RunBinaryNumberProducer, std::ref(buffer),
                    std::ref(producer_numbers), num_producer_threads, n};
    std::atomic<bool> exit{false};
    auto consumer = std::thread{RunBinaryNumberConsumer, std::ref(buffer),
                                std::ref(exit), std::ref(consumer_numbers)};
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

  SECTION("FindChunk locates the chunk that contains a given pointer.") {
    REQUIRE(AddString(buffer, "ab"));
    REQUIRE(AddString(buffer, "c"));
    buffer.Allot();
    CircularBufferConstPlacement chunk;
    int num_preceding_chunks;
    std::tie(chunk, num_preceding_chunks) =
        buffer.FindChunk(buffer.buffer().data(), buffer.buffer().data() + 9);
    REQUIRE(ToString(chunk) == "1\r\nc\r\n");
    REQUIRE(num_preceding_chunks == 1);
  }
}

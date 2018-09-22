#include "../src/message_buffer.h"
#include "../src/packet_header.h"

#include "lightstep-tracer-common/collector.pb.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;

TEST_CASE("MessageBuffer") {
  collector::Span span;
  span.set_operation_name("abc");

  static const size_t span_size = static_cast<size_t>(span.ByteSizeLong());
  static const size_t packet_size = PacketHeader::size + span_size;

  const size_t max_spans = 10;
  MessageBuffer message_buffer{packet_size * max_spans + 1};

  SECTION("Spans are correctly serialized into the buffer.") {}

  SECTION(
      "We can successfully add `max_spans` into `message_buffer` before spans "
      "are dropped.") {
    for (int i = 0; i < static_cast<int>(max_spans); ++i) {
      CHECK(message_buffer.Add(PacketType::Span, span));
    }
    CHECK(!message_buffer.Add(PacketType::Span, span));
  }

  SECTION(
      "After adding a single span, the consumer is alloted the bytes from "
      "the span.") {
    message_buffer.Add(PacketType::Span, span);
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == packet_size);
          return num_bytes;
        },
        nullptr);
  }

  SECTION(
      "After adding two spans, the consumer is alloted the bytes from both the "
      "spans.") {
    message_buffer.Add(PacketType::Span, span);
    message_buffer.Add(PacketType::Span, span);
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == 2 * packet_size);
          return num_bytes;
        },
        nullptr);
  }

  SECTION(
      "If we fill the buffer, then consume a span, we are able to fit "
      "another span.") {
    for (int i = 0; i < static_cast<int>(max_spans); ++i) {
      CHECK(message_buffer.Add(PacketType::Span, span));
    }
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == max_spans * packet_size);
          return packet_size;
        },
        nullptr);
    CHECK(message_buffer.Add(PacketType::Span, span));
    CHECK(!message_buffer.Add(PacketType::Span, span));
  }

  SECTION(
      "If the circular buffer wraps, we're only allowed to consume the largest "
      "contiguous block of memory") {
    for (int i = 0; i < static_cast<int>(max_spans); ++i) {
      CHECK(message_buffer.Add(PacketType::Span, span));
    }
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == max_spans * packet_size);
          return packet_size;
        },
        nullptr);
    CHECK(message_buffer.Add(PacketType::Span, span));
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == (max_spans - 1) * packet_size + 1);
          return num_bytes;
        },
        nullptr);
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == packet_size - 1);
          return num_bytes;
        },
        nullptr);
  }

  SECTION(
      "If we consume are partial packet, the next packet added can still be "
      "allotted to the consumer.") {
    CHECK(message_buffer.Add(PacketType::Span, span));
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == packet_size);
          return packet_size / 2;
        },
        nullptr);
    CHECK(message_buffer.Add(PacketType::Span, span));
    message_buffer.Consume(
        [](void* /*context*/, const char* /*data*/, size_t num_bytes) {
          CHECK(num_bytes == 2 * packet_size - packet_size / 2);
          return num_bytes;
        },
        nullptr);
  }
}

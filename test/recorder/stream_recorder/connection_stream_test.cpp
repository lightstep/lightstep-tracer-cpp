#include "recorder/stream_recorder/connection_stream.h"

#include <string>
#include <thread>

#include "3rd_party/catch2/catch.hpp"
#include "recorder/stream_recorder/span_stream.h"
#include "recorder/stream_recorder/utility.h"
#include "test/number_simulation.h"
#include "test/utility.h"
using namespace lightstep;

static std::string ToString(
    std::initializer_list<FragmentInputStream*> fragment_streams) {
  std::string result;
  for (auto fragment_stream : fragment_streams) {
    result += ToString(*fragment_stream);
  }
  return result;
}

static collector::ReportRequest ParseStreamHeader(std::string& s) {
  collector::ReportRequest result;
  auto body_start = s.find("\r\n\r\n");
  REQUIRE(body_start != std::string::npos);
  body_start += 4;
  auto chunk_start = s.find("\r\n", body_start);
  REQUIRE(chunk_start != std::string::npos);
  auto chunk_size =
      std::stoi(s.substr(body_start, body_start - chunk_start), nullptr, 16);
  chunk_start += 2;
  REQUIRE(result.ParseFromString(s.substr(chunk_start, chunk_size)));
  s = s.substr(chunk_start + chunk_size + 2);
  return result;
}

TEST_CASE("ConnectionStream") {
  LightStepTracerOptions tracer_options;
  ChunkCircularBuffer span_buffer{1000};
  SpanStream span_stream{span_buffer, 1};
  std::string header_common_fragment =
      WriteStreamHeaderCommonFragment(tracer_options, 123);
  StreamRecorderMetrics metrics;
  auto host_header_fragment = MakeFragment("Host:abc\r\n");
  ConnectionStream connection_stream{
      host_header_fragment,
      Fragment{static_cast<void*>(&header_common_fragment[0]),
               static_cast<int>(header_common_fragment.size())},
      metrics, span_stream};
  std::string contents;
  REQUIRE(connection_stream.Flush(
      [&contents](
          std::initializer_list<FragmentInputStream*> fragment_streams) {
        contents = ToString(fragment_streams);
        return Consume(fragment_streams, static_cast<int>(contents.size()));
      }));
  connection_stream.Reset();

  SECTION(
      "The ConnectionStream writes the same header if we flush in smaller "
      "pieces.") {
    std::string contents2;
    while (true) {
      auto result = connection_stream.Flush(
          [&contents2](
              std::initializer_list<FragmentInputStream*> fragment_streams) {
            contents2 += ToString(fragment_streams)[0];
            return Consume(fragment_streams, 1);
          });
      if (result) {
        break;
      }
    }
    REQUIRE(contents == contents2);
  }

  SECTION(
      "The ConnectionStream is only completed with the terminal fragment is "
      "written.") {
    REQUIRE(!connection_stream.completed());
    connection_stream.Flush(
        [](std::initializer_list<FragmentInputStream*> fragment_streams) {
          auto n = ToString(fragment_streams).size();
          return Consume(fragment_streams, static_cast<int>(n));
        });
    REQUIRE(!connection_stream.completed());
    connection_stream.Shutdown();
    REQUIRE(!connection_stream.completed());
    connection_stream.Flush(
        [](std::initializer_list<FragmentInputStream*> fragment_streams) {
          auto n = ToString(fragment_streams).size();
          return Consume(fragment_streams, static_cast<int>(n) - 1);
        });
    REQUIRE(!connection_stream.completed());
    connection_stream.Flush(
        [](std::initializer_list<FragmentInputStream*> fragment_streams) {
          return Consume(fragment_streams, 1);
        });
    REQUIRE(connection_stream.completed());
  }

  SECTION(
      "The ConnectionStream consumes metrics and sends them in the stream "
      "header.") {
    metrics.num_dropped_spans += 3;
    connection_stream.Reset();
    REQUIRE(metrics.num_dropped_spans == 0);
    REQUIRE(connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        }));
    auto report_request = ParseStreamHeader(contents);
    auto& counts = report_request.internal_metrics().counts();
    REQUIRE(counts.size() == 1);
    REQUIRE(counts[0].int_value() == 3);
  }

  SECTION(
      "ConnectionStream adds metrics back if it fails to send the stream "
      "header.") {
    metrics.num_dropped_spans += 3;
    connection_stream.Reset();
    REQUIRE(metrics.num_dropped_spans == 0);
    connection_stream.Flush(
        [](std::initializer_list<FragmentInputStream*> fragment_streams) {
          auto contents = ToString(fragment_streams);
          return Consume(fragment_streams,
                         static_cast<int>(contents.size()) - 1);
        });
    connection_stream.Reset();
    REQUIRE(connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        }));
    auto report_request = ParseStreamHeader(contents);
    auto& counts = report_request.internal_metrics().counts();
    REQUIRE(counts.size() == 1);
    REQUIRE(counts[0].int_value() == 3);
    REQUIRE(metrics.num_dropped_spans == 0);
  }

  SECTION(
      "After writing the header, ConnectionStream writes the contents of the "
      "span buffer.") {
    AddString(span_buffer, "abc");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        });
    ParseStreamHeader(contents);
    REQUIRE(contents == "3\r\nabc\r\n");
  }

  SECTION("If a remnant it left, it gets picked up on the next flush.") {
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        });
    AddString(span_buffer, "abc");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, 4);
        });
    AddString(span_buffer, "123");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, 4);
        });
    REQUIRE(contents == "bc\r\n3\r\n123\r\n");
  }

  SECTION(
      "If ConnectionStream is reset when there's a remnant left, it clears it "
      "and records a dropped span.") {
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        });
    AddString(span_buffer, "abc");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, 4);
        });
    connection_stream.Reset();
    AddString(span_buffer, "123");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, static_cast<int>(contents.size()));
        });
    REQUIRE(span_buffer.empty());
    auto report_request = ParseStreamHeader(contents);
    auto& counts = report_request.internal_metrics().counts();
    REQUIRE(counts.size() == 1);
    REQUIRE(counts[0].int_value() == 1);
  }
}

TEST_CASE(
    "Verify through simulation that ConnectionStream behaves correctly.") {
  LightStepTracerOptions tracer_options;
  std::string header_common_fragment =
      WriteStreamHeaderCommonFragment(tracer_options, 123);
  StreamRecorderMetrics metrics;
  auto host_header_fragment = MakeFragment("Host:abc\r\n");
  const size_t num_producer_threads = 4;
  const size_t num_connections = 10;
  const size_t n = 25000;
  for (size_t max_size : {10, 50, 100, 1000}) {
    ChunkCircularBuffer buffer{max_size};
    SpanStream span_stream{buffer, num_connections};
    std::vector<ConnectionStream> connection_streams;
    connection_streams.reserve(num_connections);
    for (int i = 0; i < static_cast<int>(num_connections); ++i) {
      connection_streams.emplace_back(
          host_header_fragment,
          Fragment{static_cast<void*>(&header_common_fragment[0]),
                   static_cast<int>(header_common_fragment.size())},
          metrics, span_stream);
    }
    std::vector<uint32_t> producer_numbers;
    std::vector<uint32_t> consumer_numbers;
    auto producer =
        std::thread{RunBinaryNumberProducer, std::ref(buffer),
                    std::ref(producer_numbers), num_producer_threads, n};
    std::atomic<bool> exit{false};
    auto consumer =
        std::thread{RunBinaryNumberConnectionConsumer, std::ref(span_stream),
                    std::ref(connection_streams), std::ref(exit),
                    std::ref(consumer_numbers)};
    producer.join();
    exit = true;
    consumer.join();
    REQUIRE(buffer.empty());
    std::sort(producer_numbers.begin(), producer_numbers.end());
    std::sort(consumer_numbers.begin(), consumer_numbers.end());
    REQUIRE(producer_numbers == consumer_numbers);
  }
}
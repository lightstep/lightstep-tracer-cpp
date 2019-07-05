#include "recorder/stream_recorder/connection_stream2.h"

#include "3rd_party/catch2/catch.hpp"
#include "recorder/stream_recorder/span_stream2.h"
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
  CircularBuffer2<SerializationChain> span_buffer{1000};
  MetricsObserver metrics_observer;
  StreamRecorderMetrics metrics{metrics_observer};
  SpanStream2 span_stream{span_buffer, metrics};
  std::string header_common_fragment =
      WriteStreamHeaderCommonFragment(tracer_options, 123);
  auto host_header_fragment = MakeFragment("Host:abc\r\n");
  ConnectionStream2 connection_stream{
      host_header_fragment,
      Fragment{static_cast<void*>(&header_common_fragment[0]),
               static_cast<int>(header_common_fragment.size())},
      span_stream};
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
    metrics.OnSpansDropped(3);
    connection_stream.Reset();
    REQUIRE(metrics.num_dropped_spans() == 0);
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
    metrics.OnSpansDropped(3);
    connection_stream.Reset();
    REQUIRE(metrics.num_dropped_spans() == 0);
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
    REQUIRE(metrics.num_dropped_spans() == 0);
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
    REQUIRE(contents == AddSpanChunkFraming("abc"));
  }

  SECTION("If a remnant is left, it gets picked up on the next flush.") {
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
    REQUIRE(contents == (AddSpanChunkFraming("abc") + AddSpanChunkFraming("123")).substr(4));
  }

  SECTION(
      "If a remnant is left when the stream is shutdown, it still gets "
      "written.") {
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
    connection_stream.Shutdown();
    AddString(span_buffer, "123");
    connection_stream.Flush(
        [&contents](
            std::initializer_list<FragmentInputStream*> fragment_streams) {
          contents = ToString(fragment_streams);
          return Consume(fragment_streams, 4);
        });
    REQUIRE(contents == AddSpanChunkFraming("abc").substr(4) + "0\r\n\r\n");
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

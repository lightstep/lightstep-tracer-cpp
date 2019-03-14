#include "recorder/stream_recorder/connection_stream.h"

#include "3rd_party/catch2/catch.hpp"
#include "recorder/stream_recorder/span_stream.h"
#include "recorder/stream_recorder/utility.h"
#include "test/network/utility.h"
using namespace lightstep;

static std::string ToString(
    std::initializer_list<FragmentInputStream*> fragment_streams) {
  std::string result;
  for (auto fragment_stream : fragment_streams) {
    result += ToString(*fragment_stream);
  }
  return result;
}

TEST_CASE("ConnectionStream") {
  LightStepTracerOptions tracer_options;
  ChunkCircularBuffer span_buffer{1000};
  SpanStream span_stream{span_buffer};
  std::string header_common_fragment =
      WriteStreamHeaderCommonFragment(tracer_options, 123);
  StreamRecorderMetrics metrics;
  auto host_header_fragment = MakeFragment("Host:abc\r\n");
  ConnectionStream connection_stream{
      host_header_fragment, MakeFragment(header_common_fragment.c_str()),
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
    while (1) {
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
}

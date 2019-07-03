#include "recorder/stream_recorder/span_stream2.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SpanStream") {
  const size_t max_spans = 10;
  CircularBuffer2<SerializationChain> buffer{max_spans};
  MetricsObserver metrics_observer;
  StreamRecorderMetrics metrics{metrics_observer};
  SpanStream2 span_stream{buffer, metrics};

  SECTION("When the attached buffer is empty, SpanStream has no fragments") {
    REQUIRE(span_stream.empty());
  }
}

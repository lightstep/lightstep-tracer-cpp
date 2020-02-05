#include "recorder/stream_recorder/span_stream.h"

#include <iostream>

#include "test/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SpanStream") {
  const size_t max_spans = 10;
  CircularBuffer<ChainedStream> buffer{max_spans};
  MetricsObserver metrics_observer;
  MetricsTracker metrics{metrics_observer};
  SpanStream span_stream{buffer, metrics};

  SECTION("When the attached buffer is empty, SpanStream has no fragments") {
    REQUIRE(span_stream.empty());
    span_stream.Allot();
    REQUIRE(span_stream.empty());
  }

  SECTION("SpanStream mirrors the contents of its attached buffer") {
    REQUIRE(AddSpanChunkFramedString(buffer, "abc123"));
    span_stream.Allot();
    REQUIRE(ToString(span_stream) == AddSpanChunkFraming("abc123"));
  }

  SECTION("SpanStream is empty after it's been cleared") {
    REQUIRE(AddSpanChunkFramedString(buffer, "abc123"));
    span_stream.Allot();
    span_stream.Clear();
    REQUIRE(span_stream.empty());
  }

  SECTION("SpanStream leaves a remnant if a span is partially consumed") {
    REQUIRE(AddSpanChunkFramedString(buffer, "abc123"));
    span_stream.Allot();
    auto contents = ToString(span_stream);
    REQUIRE(!Consume({&span_stream}, 3));
    auto remnant = span_stream.ConsumeRemnant();
    REQUIRE(remnant != nullptr);
    REQUIRE(span_stream.ConsumeRemnant() == nullptr);
    REQUIRE(ToString(*remnant) == contents.substr(3));
  }

  SECTION("SpanStream leaves no remnant when spans are completely consumed") {
    REQUIRE(AddSpanChunkFramedString(buffer, "abc123"));
    span_stream.Allot();
    span_stream.Clear();
    REQUIRE(span_stream.ConsumeRemnant() == nullptr);

    REQUIRE(AddSpanChunkFramedString(buffer, "abc"));
    REQUIRE(AddSpanChunkFramedString(buffer, "123"));
    span_stream.Allot();
    auto span1 = AddSpanChunkFraming("abc");
    REQUIRE(!Consume({&span_stream}, static_cast<int>(span1.size())));
    REQUIRE(span_stream.ConsumeRemnant() == nullptr);
    span_stream.Allot();
    REQUIRE(ToString(span_stream) == AddSpanChunkFraming("123"));
  }
}

#include "recorder/stream_recorder/span_stream.h"

#include "test/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("SpanStream") {
  const int max_connections = 10;
  const int max_buffer_bytes = 20;
  ChunkCircularBuffer buffer{max_buffer_bytes};
  SpanStream span_stream{buffer, 10};

  SECTION("When the attached buffer is empty, SpanStream has no fragments.") {
    REQUIRE(span_stream.empty());
  }

  SECTION("SpanStream contains a single fragment if the buffer doesn't wrap.") {
    AddString(buffer, "abc");
    span_stream.Allot();
    REQUIRE(span_stream.num_fragments() == 1);
    REQUIRE(ToString(span_stream) == "3\r\nabc\r\n");
  }

  SECTION("If a chunk is partially consumed, we advance to the end of the chunk and store a remnant.") {
    AddString(buffer, "abc");
    AddString(buffer, "123");
    span_stream.Allot();
    Consume({&span_stream}, 4);
    /* REQUIRE(ToString(span_stream) == "3\r\n123\r\n"); */
  }
}

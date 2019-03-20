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

  SECTION("SpanStream's position is correctly updated if the buffer fills.") {
    AddString(buffer, "X");
    span_stream.Allot();
    Consume({&span_stream}, 6);
    span_stream.Consume();
    REQUIRE(buffer.empty());

    AddString(buffer, std::string{10, 'X'});
    span_stream.Allot();
    Consume({&span_stream}, 15);
    span_stream.Consume();
    REQUIRE(buffer.empty());

    AddString(buffer, "abc");
    span_stream.Allot();
    REQUIRE(ToString(span_stream) == "3\r\nabc\r\n");
  }

  SECTION(
      "If a chunk is partially consumed, we advance to the end of the chunk "
      "and store a remnant.") {
    AddString(buffer, "abc");
    AddString(buffer, "123");
    span_stream.Allot();
    Consume({&span_stream}, 4);
    REQUIRE(ToString(span_stream) == "3\r\n123\r\n");
    FragmentSpanInputStream span_remnant;
    span_stream.PopSpanRemnant(span_remnant);
    REQUIRE(ToString(span_remnant) == "bc\r\n");
  }

  SECTION("If chunks are fully consumed, there's no remnant.") {
    AddString(buffer, "abc");
    AddString(buffer, "123");
    span_stream.Allot();
    Consume({&span_stream}, 8);
    REQUIRE(ToString(span_stream) == "3\r\n123\r\n");
    FragmentSpanInputStream span_remnant;
    span_stream.PopSpanRemnant(span_remnant);
    REQUIRE(span_remnant.empty());
  }

  SECTION("SpanStream won't consume past a remnant until it's been cleared.") {
    AddString(buffer, "abc");
    span_stream.Allot();
    Consume({&span_stream}, 4);
    FragmentSpanInputStream span_remnant1;
    span_stream.PopSpanRemnant(span_remnant1);
    span_stream.Consume();

    AddString(buffer, "123");
    span_stream.Allot();
    Consume({&span_stream}, 8);
    FragmentSpanInputStream span_remnant2;
    span_stream.PopSpanRemnant(span_remnant2);
    REQUIRE(span_remnant2.empty());
    span_stream.Consume();

    REQUIRE(ToString(buffer.allotment()) == "3\r\nabc\r\n3\r\n123\r\n");
    span_stream.RemoveRemnant(span_remnant1.chunk_start());
    span_stream.Consume();
    REQUIRE(ToString(buffer.allotment()).empty());
  }
}

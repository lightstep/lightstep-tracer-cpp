#include "recorder/stream_recorder/fragment_span_input_stream.h"

#include <cstdint>

#include "3rd_party/catch2/catch.hpp"
#include "test/utility.h"
using namespace lightstep;

TEST_CASE("FragmentSpanInputStream") {
  FragmentSpanInputStream stream;
  const char* s1 = "abc";
  const char* s2 = "123";

  REQUIRE(stream.empty());
  REQUIRE(stream.chunk_start() == nullptr);

  SECTION("Set adds fragments for a span.") {
    CircularBufferConstPlacement placement{s1, 2, s2, 3};
    stream.Set(placement, s1 + 1);
    REQUIRE(ToString(stream) == "b123");
  }
}

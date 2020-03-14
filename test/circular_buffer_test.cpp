#include "test/circular_buffer.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  CircularBuffer<int> buffer{5};
  int count = 0;

  SECTION("The default CircularBuffer is empty") {
    buffer.Consume([&](std::unique_ptr<int>&& /*ptr*/) { ++count; });
    REQUIRE(count == 0);
  }
}

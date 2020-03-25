#include "test/baseline_circular_buffer.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  size_t max_elements = 5;
  BaselineCircularBuffer<int> buffer{max_elements};
  int count = 0;

  SECTION("The default CircularBuffer is empty") {
    buffer.Consume([&](std::unique_ptr<int>&& /*ptr*/) { ++count; });
    REQUIRE(count == 0);
  }

  SECTION("We can consume elements out of the circular buffer") {
    REQUIRE(buffer.Add(std::unique_ptr<int>{new int{123}}));
    buffer.Consume([&](std::unique_ptr<int>&& ptr) {
      REQUIRE(*ptr == 123);
      ++count;
    });
    REQUIRE(count == 1);
  }

  SECTION("Add fails if the buffer is full") {
    for (size_t i = 0; i < max_elements; ++i) {
      REQUIRE(buffer.Add(std::unique_ptr<int>{new int{123}}));
    }
    REQUIRE(!buffer.Add(std::unique_ptr<int>{new int{123}}));
  }

  SECTION("We can consume from a wrapped buffer") {
    REQUIRE(buffer.Add(std::unique_ptr<int>{new int{123}}));
    REQUIRE(buffer.Add(std::unique_ptr<int>{new int{123}}));
    buffer.Consume([&](std::unique_ptr<int>&& /*ptr*/) {});
    for (size_t i = 0; i < max_elements - 1; ++i) {
      REQUIRE(buffer.Add(std::unique_ptr<int>{new int{123}}));
    }
    buffer.Consume([&](std::unique_ptr<int>&& /*ptr*/) { ++count; });
    REQUIRE(count == max_elements - 1);
  }
}

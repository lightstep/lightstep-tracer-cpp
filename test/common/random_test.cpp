#include "common/random.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("Random") {
  SECTION("GenerateId returns a random 64-bit number.") {
    auto x = GenerateId();
    auto y = GenerateId();
    REQUIRE(x != y);
  }

  SECTION(
      "GeneateRandomDuration returns a random duration within a given range.") {
    auto a = std::chrono::microseconds{5};
    auto b = std::chrono::microseconds{10};
    auto t = GenerateRandomDuration(a, b);
    REQUIRE(a <= t);
    REQUIRE(t <= b);
  }
}

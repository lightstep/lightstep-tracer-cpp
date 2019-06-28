#include "common/circular_buffer_range.h"

#include <iterator>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBufferRange") {
  int array1[] = {1, 2, 3, 4};
  int array2[] = {5, 6, 7};
  CircularBufferRange<int> range{std::begin(array1), std::end(array1),
                                 std::begin(array2), std::end(array2)};

  SECTION("We can iterate over elements of a CircularBufferRange") {
    int x = 0;
    range.ForEach([&](int y) {
      REQUIRE(++x == y);
      return true;
    });
    REQUIRE(x == 7);
  }

  SECTION("We can bail out of iteration over a CircularBufferRange") {
    int x = 0;
    range.ForEach([&](int y) {
      REQUIRE(++x == y);
      return false;
    });
    REQUIRE(x == 1);

    x = 0;
    range.ForEach([&](int y) {
      REQUIRE(++x == y);
      return y != 5;
    });
    REQUIRE(x == 5);
  }

  SECTION(
      "We can implicitly convert a non-const CircularBufferRange to a const "
      "one") {
    CircularBufferRange<const int> range2{range};
    int x = 0;
    range2.ForEach([&](int y) {
      REQUIRE(++x == y);
      return true;
    });
    REQUIRE(x == 7);
  }
}

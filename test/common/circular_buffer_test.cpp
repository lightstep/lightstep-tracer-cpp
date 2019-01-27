#include "common/circular_buffer.h"

#include <array>
#include <utility>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  CircularBuffer circular_buffer{5};
  REQUIRE(circular_buffer.max_size() == 5);
  REQUIRE(circular_buffer.empty());

  SECTION(
      "We can make a single reservation that doesn't exceed the capacity.") {
    auto reservation = circular_buffer.Reserve(2);
    REQUIRE(reservation.data1 != nullptr);
    REQUIRE(reservation.size1 == 2);
    REQUIRE(reservation.size2 == 0);

    auto placement = circular_buffer.Peek(0, 2);
    REQUIRE(placement.data1 == reservation.data1);
  }

  SECTION(
      "Sequential reservations that don't exceed the capacity are placed "
      "sequentially.") {
    auto reservation1 = circular_buffer.Reserve(1);
    REQUIRE(reservation1.data1 != nullptr);
    REQUIRE(reservation1.size1 == 1);
    reservation1.data1[0] = 'a';

    auto reservation2 = circular_buffer.Reserve(2);
    REQUIRE(reservation2.data1 != nullptr);
    REQUIRE(reservation2.size1 == 2);

    reservation2.data1[0] = 'b';
    reservation2.data1[1] = 'c';

    REQUIRE(circular_buffer.size() == 3);

    auto placement = circular_buffer.Peek(0, 3);
    REQUIRE(placement.size1 == 3);
    CHECK(placement.data1[0] == 'a');
    CHECK(placement.data1[1] == 'b');
    CHECK(placement.data1[2] == 'c');
  }

  SECTION(
      "Reserve fails if the amount of space requested is more than what's "
      "available in the circular buffer.") {
    REQUIRE(circular_buffer.Reserve(6).data1 == nullptr);

    REQUIRE(circular_buffer.Reserve(2).data2 != nullptr);
    REQUIRE(circular_buffer.size() == 2);

    REQUIRE(circular_buffer.Reserve(3).data2 != nullptr);
    REQUIRE(circular_buffer.size() == 5);

    REQUIRE(circular_buffer.Reserve(1).data2 == nullptr);
    REQUIRE(circular_buffer.size() == 5);
  }

  SECTION("Reservations can wrap around the end of a circular buffer.") {
    REQUIRE(circular_buffer.Reserve(3).data2 != nullptr);
    REQUIRE(circular_buffer.size() == 3);
    circular_buffer.Consume(3);
    REQUIRE(circular_buffer.empty());

    auto placement = circular_buffer.Reserve(4);
    REQUIRE(placement.size1 == 3);
    REQUIRE(placement.size2 == 1);
    REQUIRE(placement.data1 == placement.data2 + 3);
  }
}

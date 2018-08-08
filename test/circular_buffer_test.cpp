#include "../src/circular_buffer.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch2/catch.hpp>
using namespace lightstep;

TEST_CASE("CircularBuffer") {
  CircularBuffer circular_buffer{5};

  SECTION("Make single reservation that doesn't exceed capacity") {
    auto reservation = circular_buffer.Reserve(2);
    REQUIRE(reservation.data1 != nullptr);
    REQUIRE(reservation.size1 == 2);
    REQUIRE(reservation.size2 == 0);
    REQUIRE(circular_buffer.tail_data() == reservation.data1);
  }

  SECTION(
      "Verify multiple reservations that don't exceed capacity are "
      "contiguous") {
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

    auto data = circular_buffer.tail_data();
    CHECK(data[0] == 'a');
    CHECK(data[1] == 'b');
    CHECK(data[2] == 'c');
  }
}

#include "common/atomic_bit_set.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("AtomicBitSet") {
  size_t size = 100;
  AtomicBitSet bit_set{size};

  SECTION("A bit set is initialized with everything set to false.") {
    for (int bit_index = 0; bit_index < static_cast<int>(size); ++bit_index) {
      REQUIRE(!bit_set.Test(bit_index));
    }
  }

  SECTION("After calling set a bit tests as true.") {
    bit_set.Set(63);
    for (int bit_index = 0; bit_index < static_cast<int>(size); ++bit_index) {
      if (bit_index == 63) {
        REQUIRE(bit_set.Test(bit_index));
      } else {
        REQUIRE(!bit_set.Test(bit_index));
      }
    }
  }

  SECTION("Setting a bit in the second block works as expected") {
    CHECK(!bit_set.Set(64));
    CHECK(bit_set.Set(64));
    for (int bit_index = 0; bit_index < static_cast<int>(size); ++bit_index) {
      if (bit_index == 64) {
        REQUIRE(bit_set.Test(bit_index));
      } else {
        REQUIRE(!bit_set.Test(bit_index));
      }
    }
  }

  SECTION("Set, then Reset returns everything to zero") {
    bit_set.Set(63);
    CHECK(bit_set.Reset(63));
    CHECK(!bit_set.Reset(63));
    for (int bit_index = 0; bit_index < static_cast<int>(size); ++bit_index) {
      REQUIRE(!bit_set.Test(bit_index));
    }
  }
}

#include "common/atomic_unique_ptr.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("AtomicUniquePtr") {
  AtomicUniquePtr<int> ptr;
  REQUIRE(ptr.IsNull());

  SECTION("SwapIfNull swaps an AtomicUniquePtr is null") {
    std::unique_ptr<int> x{new int{33}};
    REQUIRE(ptr.SwapIfNull(x));
    REQUIRE(x == nullptr);
    REQUIRE(*ptr == 33);
  }

  SECTION("SwapIfNull does nothing if AtomicUniquePtr is non-null") {
    ptr.Reset(new int{11});
    std::unique_ptr<int> x{new int{33}};
    REQUIRE(!ptr.SwapIfNull(x));
    REQUIRE(x != nullptr);
    REQUIRE(*x == 33);
    REQUIRE(*ptr == 11);
  }

  SECTION("Swap always swaps an AtomicUniquePtr") {
    ptr.Reset(new int{11});
    std::unique_ptr<int> x{new int{33}};
    ptr.Swap(x);
    REQUIRE(!ptr.IsNull());
    REQUIRE(x != nullptr);
    REQUIRE(*x == 11);
    REQUIRE(*ptr == 33);
  }
}

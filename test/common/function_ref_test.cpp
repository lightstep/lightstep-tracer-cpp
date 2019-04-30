#include "common/function_ref.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

int Call(FunctionRef<int()> f) { return f(); }

int Return3() { return 3; }

TEST_CASE("FunctionRef") {
  int x = 9;

  auto f = [&] { return x; };

  SECTION("FunctionRef can reference a lambda functor.") {
    REQUIRE(Call(f) == 9);
  }

  SECTION("FunctionRef can reference a function pointer.") {
    REQUIRE(Call(Return3) == 3);
  }

  SECTION("FunctionRef can be converted to bool") {
    FunctionRef<int()> fref1{nullptr};
    FunctionRef<int()> fref2{f};
    REQUIRE(!static_cast<bool>(fref1));
    REQUIRE(static_cast<bool>(fref2));
  }
}

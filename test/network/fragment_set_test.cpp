#include "network/fragment_set.h"
#include "3rd_party/catch2/catch.hpp"
#include "test/network/utility.h"
using namespace lightstep;

TEST_CASE("FragmentSet") {
  auto fragment_set1 = MakeFixedFragmentInputStream("abc", "123");
  auto fragment_set2 = MakeFixedFragmentInputStream("alpha", "beta", "gamma");
  std::initializer_list<const FragmentSet*> fragment_sets = {&fragment_set1,
                                                             &fragment_set2};
  REQUIRE(ComputeFragmentPosition(fragment_sets, 0) ==
          std::make_tuple(0, 0, 0));
  REQUIRE(ComputeFragmentPosition(fragment_sets, 1) ==
          std::make_tuple(0, 0, 1));
  REQUIRE(ComputeFragmentPosition(fragment_sets, 3) ==
          std::make_tuple(0, 1, 0));
  REQUIRE(ComputeFragmentPosition(fragment_sets, 4) ==
          std::make_tuple(0, 1, 1));
  REQUIRE(ComputeFragmentPosition(fragment_sets, 11) ==
          std::make_tuple(1, 1, 0));
  REQUIRE(ComputeFragmentPosition(fragment_sets, 20) ==
          std::make_tuple(2, 0, 0));
}

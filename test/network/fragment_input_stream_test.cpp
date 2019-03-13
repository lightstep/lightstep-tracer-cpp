#include "network/fragment_input_stream.h"
#include "3rd_party/catch2/catch.hpp"
#include "test/network/utility.h"
using namespace lightstep;

TEST_CASE("FragmentInputStream") {
  auto fragment_input_stream1 = MakeFixedFragmentInputStream("abc", "123");
  auto fragment_input_stream2 =
      MakeFixedFragmentInputStream("alpha", "beta", "gamma");
  std::initializer_list<const FragmentInputStream*> fragment_input_streams = {
      &fragment_input_stream1, &fragment_input_stream2};
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 0) ==
          std::make_tuple(0, 0, 0));
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 1) ==
          std::make_tuple(0, 0, 1));
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 3) ==
          std::make_tuple(0, 1, 0));
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 4) ==
          std::make_tuple(0, 1, 1));
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 11) ==
          std::make_tuple(1, 1, 0));
  REQUIRE(ComputeFragmentPosition(fragment_input_streams, 20) ==
          std::make_tuple(2, 0, 0));
}

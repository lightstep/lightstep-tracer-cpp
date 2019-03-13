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

TEST_CASE("FragmentInputStream2") {
  auto fragment_input_stream1 = MakeFixedFragmentInputStream("abc", "123");
  auto fragment_input_stream2 =
      MakeFixedFragmentInputStream("alpha", "beta", "gamma");
  std::initializer_list<FragmentInputStream*> fragment_input_streams = {
      &fragment_input_stream1, &fragment_input_stream2};

  SECTION("Consuming nothing leaves the streams unmodified.") {
    REQUIRE(!Consume(fragment_input_streams, 0));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "abc123alphabetagamma");
  }

  SECTION("We can consume bytes from the first fragment.") {
    REQUIRE(!Consume(fragment_input_streams, 1));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "bc123alphabetagamma");
  }

  SECTION("We can consume the first fragment.") {
    REQUIRE(!Consume(fragment_input_streams, 3));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "123alphabetagamma");
  }

  SECTION("We can consume into the second fragment.") {
    REQUIRE(!Consume(fragment_input_streams, 4));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "23alphabetagamma");
  }

  SECTION("We can consume first fragment set.") {
    REQUIRE(!Consume(fragment_input_streams, 6));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "alphabetagamma");
  }

  SECTION("We can consume into the second fragment set.") {
    REQUIRE(!Consume(fragment_input_streams, 11));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "betagamma");

    REQUIRE(!Consume(fragment_input_streams, 1));

    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "etagamma");
  }

  SECTION("We can consume all the data.") {
    REQUIRE(Consume(fragment_input_streams, 20));
    REQUIRE((ToString(fragment_input_stream1) +
             ToString(fragment_input_stream2)) == "");
  }
}

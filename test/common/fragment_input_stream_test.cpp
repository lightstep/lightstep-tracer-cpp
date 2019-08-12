#include <random>

#include "3rd_party/catch2/catch.hpp"
#include "common/fragment_array_input_stream.h"
#include "test/utility.h"
using namespace lightstep;

TEST_CASE("FragmentInputStream") {
  FragmentArrayInputStream fragment_input_stream1{MakeFragment("abc"),
                                                  MakeFragment("123")};
  FragmentArrayInputStream fragment_input_stream2{
      MakeFragment("alpha"), MakeFragment("beta"), MakeFragment("gamma")};
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
    REQUIRE(
        (ToString(fragment_input_stream1) + ToString(fragment_input_stream2))
            .empty());
  }

  SECTION("We can consume the data one byte at a time.") {
    std::string s;
    while (true) {
      s += (ToString(fragment_input_stream1) +
            ToString(fragment_input_stream2))[0];
      auto result = Consume(fragment_input_streams, 1);
      if (result) {
        break;
      }
    }
    REQUIRE(s == "abc123alphabetagamma");
  }

  SECTION("We can consume data of random lengths.") {
    std::mt19937 random_number_generator{std::random_device{}()};
    for (int i = 0; i < 100; ++i) {
      fragment_input_stream1 = {MakeFragment("abc"), MakeFragment("123")};
      fragment_input_stream2 = {MakeFragment("alpha"), MakeFragment("beta"),
                                MakeFragment("gamma")};
      std::string s;
      while (true) {
        auto contents =
            ToString(fragment_input_stream1) + ToString(fragment_input_stream2);
        std::uniform_int_distribution<size_t> distribution{0, contents.size()};
        auto n = distribution(random_number_generator);
        s += contents.substr(0, n);
        auto result = Consume(fragment_input_streams, static_cast<int>(n));
        if (result) {
          break;
        }
      }
      REQUIRE(s == "abc123alphabetagamma");
    }
  }
}

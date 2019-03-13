#include "network/fixed_fragment_input_stream.h"

#include <cstring>
#include <string>

#include "3rd_party/catch2/catch.hpp"
#include "test/network/utility.h"
using namespace lightstep;

TEST_CASE("FixedFragmentInputStream") {
  auto input_stream = MakeFixedFragmentInputStream("abc", "123");
  REQUIRE(input_stream.num_fragments() == 2);
  REQUIRE(ToString(input_stream) == "abc123");

  SECTION("FixedFragmentInputStream default constructs to an empty stream.") {
    FixedFragmentInputStream<10> input_stream2;
    REQUIRE(input_stream2.empty());
  }

  SECTION(
      "FixedFragmentInputStream can hold fewer fragments than its capacity.") {
    FixedFragmentInputStream<10> input_stream2;
    input_stream2 = {Fragment{static_cast<void*>(const_cast<char*>("abc")), 3},
                     Fragment{static_cast<void*>(const_cast<char*>("123")), 3}};
    REQUIRE(input_stream2.num_fragments() == 2);
    REQUIRE(ToString(input_stream2) == "abc123");
  }

  SECTION("We can seek past characters in the fragment stream.") {
    input_stream.Seek(0, 1);
    REQUIRE(input_stream.num_fragments() == 2);
    REQUIRE(ToString(input_stream) == "bc123");
  }

  SECTION("We can seek past a fragment.") {
    input_stream.Seek(1, 0);
    REQUIRE(input_stream.num_fragments() == 1);
    REQUIRE(ToString(input_stream) == "123");
  }

  SECTION("Seeks are done relative to the current position.") {
    input_stream.Seek(0, 1);
    input_stream.Seek(0, 1);
    REQUIRE(input_stream.num_fragments() == 2);
    REQUIRE(ToString(input_stream) == "c123");
    input_stream.Seek(1, 0);
    input_stream.Seek(0, 1);
    REQUIRE(input_stream.num_fragments() == 1);
    REQUIRE(ToString(input_stream) == "23");
  }

  SECTION("When we Clear a stream, there's nothing in it.") {
    input_stream.Clear();
    REQUIRE(input_stream.num_fragments() == 0);
    int fragment_count = 0;
    input_stream.ForEachFragment(
        [&fragment_count](void* /*data*/, int /*size*/) {
          ++fragment_count;
          return true;
        });
    REQUIRE(fragment_count == 0);
  }
}

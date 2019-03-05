#include "network/fragment_input_stream.h"

#include <cstring>
#include <string>

#include "3rd_party/catch2/catch.hpp"
#include "test/network/utility.h"
using namespace lightstep;

TEST_CASE("FragmentInputStream") {
  auto input_stream = MakeFragmentInputStream("abc", "123");
  REQUIRE(input_stream.num_fragments() == 2);
  REQUIRE(ToString(input_stream) == "abc123");

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

  SECTION("When we Clear a stream, there's nothing in it.") {
    input_stream.Clear();
    REQUIRE(input_stream.num_fragments() == 0);
    int fragment_count = 0;
    input_stream.ForEachFragment(
        [](void* /*data*/, int /*size*/, void* context) {
          ++*static_cast<int*>(context);
          return true;
        },
        static_cast<void*>(&fragment_count));
    REQUIRE(fragment_count == 0);
  }
}

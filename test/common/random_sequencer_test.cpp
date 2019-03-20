#include "common/random_sequencer.h"

#include <algorithm>
#include <vector>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("RandomSequencer") {
  std::vector<int> v(5, 0);
  RandomSequencer sequencer{static_cast<int>(v.size())};

  SECTION(
      "RandomSequencer allows iteration over sequential indexes in random "
      "order.") {
    auto completed = sequencer.ForEachIndex([&v](int i) {
      ++v[i];
      return true;
    });
    REQUIRE(completed);
    REQUIRE(std::all_of(v.begin(), v.end(), [](int x) { return x == 1; }));
  }

  SECTION("Iteration can be exited early by returning false.") {
    auto completed = sequencer.ForEachIndex([&v](int i) {
      ++v[i];
      return false;
    });
    REQUIRE(!completed);
    std::sort(v.begin(), v.end());
    REQUIRE(v == std::vector<int>{0, 0, 0, 0, 1});
  }

  SECTION("The callback is never called if the sequence is empty.") {
    sequencer = RandomSequencer{0};
    auto completed = sequencer.ForEachIndex([](int /*i*/) {
      REQUIRE(false);
      return true;
    });
    REQUIRE(completed);
  }
}

#include "common/random_traverser.h"

#include <algorithm>
#include <vector>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("RandomTraverser") {
  std::vector<int> v(5, 0);
  RandomTraverser traverser{static_cast<int>(v.size())};

  SECTION(
      "RandomTraverser allows iteration over sequential indexes in random "
      "order.") {
    auto completed = traverser.ForEachIndex([&v](int i) {
      ++v[i];
      return true;
    });
    REQUIRE(completed);
    REQUIRE(std::all_of(v.begin(), v.end(), [](int x) { return x == 1; }));
  }

  SECTION("Iteration can be exited early by returning false.") {
    auto completed = traverser.ForEachIndex([&v](int i) {
      ++v[i];
      return false;
    });
    REQUIRE(!completed);
    std::sort(v.begin(), v.end());
    REQUIRE(v == std::vector<int>{0, 0, 0, 0, 1});
  }

  SECTION("The callback is never called if the sequence is empty.") {
    traverser = RandomTraverser{0};
    auto completed = traverser.ForEachIndex([](int /*i*/) {
      REQUIRE(false);
      return true;
    });
    REQUIRE(completed);
  }
}

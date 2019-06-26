#include "common/flat_map.h"

#include <algorithm>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("FlatMap") {
  FlatMap<int, int> map;
  REQUIRE(map.empty());

  SECTION("We can insert_or_assign values into map") {
    map.insert_or_assign(3, 4);
    auto iter = map.find(3);
    REQUIRE(iter != map.end());
    REQUIRE(iter->second == 4);
  }

  SECTION("We can replace a value already inserted") {
    map.insert_or_assign(3, 4);
    map.insert_or_assign(3, 7);
    auto iter = map.find(3);
    REQUIRE(iter != map.end());
    REQUIRE(iter->second == 7);
  }

  SECTION("find returns the last iterator if the value isn't present") {
    REQUIRE(map.find(22) == map.end());
    map.insert_or_assign(3, 4);
    REQUIRE(map.find(22) == map.end());
  }

  SECTION("FlatMap keeps keys in sorted order") {
    for (int key : {3, 9, -1, 4, 5, 0, 10, 2}) {
      map.insert_or_assign(std::move(key), 33);
      auto comp = [](const std::pair<int, int>& lhs,
                     const std::pair<int, int>& rhs) {
        return lhs.first < rhs.first;
      };
      REQUIRE(std::is_sorted(map.begin(), map.end(), comp));
    }
  }
}

#include "common/flat_map.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("FlatMap") {
  FlatMap<int, int> map;
  (void)map;
}

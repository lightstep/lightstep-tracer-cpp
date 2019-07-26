#include "common/fast_random_number_generator.h"

#include <random>
#include <set>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("FastRandomNumberGenerator") {
  std::seed_seq seed_sequence{1, 2, 3};
  FastRandomNumberGenerator random_number_generator;
  random_number_generator.seed(seed_sequence);

  SECTION(
      "If seededed, we can expect FastRandomNumberGenerator to have a long "
      "period before it repeats itself") {
    std::set<uint64_t> values;
    for (int i = 0; i < 1000; ++i) {
      REQUIRE(values.insert(random_number_generator()).second);
    }
  }
}

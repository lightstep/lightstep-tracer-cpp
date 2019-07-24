#include "common/timestamp.h"

#include <cmath>

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE(
    "A system-steady timestamp delta can be used to convert between the "
    "different timestamps") {
  auto timestamp_delta = ComputeSystemSteadyTimestampDelta();
  auto system_now = std::chrono::system_clock::now();
  auto steady_now = std::chrono::steady_clock::now();

  auto system_now_prime = ToSystemTimestamp(timestamp_delta, steady_now);
  auto steady_now_prime = ToSteadyTimestamp(timestamp_delta, system_now);

  REQUIRE(std::abs(std::chrono::duration_cast<std::chrono::microseconds>(
                       system_now_prime - system_now)
                       .count()) < 100);

  REQUIRE(std::abs(std::chrono::duration_cast<std::chrono::microseconds>(
                       steady_now_prime - steady_now)
                       .count()) < 100);
}

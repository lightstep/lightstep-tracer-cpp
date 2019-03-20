#pragma once

#include <chrono>
#include <cstdint>
#include <random>

namespace lightstep {
std::mt19937_64& GetRandomNumberGenerator();

/**
 * @return a random 64-bit number.
 */
uint64_t GenerateId();

/**
 * Uniformily generates a random duration within a given range.
 * @param a supplies the smallest duration to generate.
 * @param b supplies the largest duration to generate.
 * @return a random duration within [a,b]
 */
std::chrono::nanoseconds GenerateRandomDuration(std::chrono::nanoseconds a,
                                                std::chrono::nanoseconds b);

template <class Rep, class Period>
std::chrono::duration<Rep, Period> GenerateRandomDuration(
    std::chrono::duration<Rep, Period> a,
    std::chrono::duration<Rep, Period> b) {
  return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
      GenerateRandomDuration(
          std::chrono::duration_cast<std::chrono::nanoseconds>(a),
          std::chrono::duration_cast<std::chrono::nanoseconds>(b)));
}
}  // namespace lightstep

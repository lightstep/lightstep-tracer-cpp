#pragma once

#include <chrono>
#include <cstdint>
#include <random>

#include "common/fast_random_number_generator.h"

namespace lightstep {
/**
 * @return a seeded thread-local random number generator.
 */
FastRandomNumberGenerator& GetRandomNumberGenerator() noexcept;

/**
 * @return a random 64-bit number.
 */
inline uint64_t GenerateId() noexcept { return GetRandomNumberGenerator()(); }

/**
 * Provides a more performant way to generate multiple ids.
 * @return an array of random 64-bit numbers.
 */
template <size_t N>
inline std::array<uint64_t, N> GenerateIds() noexcept {
  auto& random_number_generator = GetRandomNumberGenerator();
  std::array<uint64_t, N> result;
  for (auto& value : result) {
    value = random_number_generator();
  }
  return result;
}

/**
 * Uniformily generates a random duration within a given range.
 * @param a supplies the smallest duration to generate.
 * @param b supplies the largest duration to generate.
 * @return a random duration within [a,b]
 */
std::chrono::nanoseconds GenerateRandomDuration(
    std::chrono::nanoseconds a, std::chrono::nanoseconds b) noexcept;

template <class Rep, class Period>
std::chrono::duration<Rep, Period> GenerateRandomDuration(
    std::chrono::duration<Rep, Period> a,
    std::chrono::duration<Rep, Period> b) noexcept {
  return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
      GenerateRandomDuration(
          std::chrono::duration_cast<std::chrono::nanoseconds>(a),
          std::chrono::duration_cast<std::chrono::nanoseconds>(b)));
}
}  // namespace lightstep

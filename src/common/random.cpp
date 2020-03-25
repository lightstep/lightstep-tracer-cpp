#include "common/random.h"

#include <cassert>

#include "common/platform/fork.h"

namespace lightstep {
//------------------------------------------------------------------------------
// TlsRandomNumberGenerator
//------------------------------------------------------------------------------
// Wraps a thread_local random number generator, but adds a fork handler so that
// the generator will be correctly seeded after forking.
//
// See https://stackoverflow.com/q/51882689/4447365 and
//     https://github.com/opentracing-contrib/nginx-opentracing/issues/52
namespace {
class TlsRandomNumberGenerator {
 public:
  TlsRandomNumberGenerator() {
    Seed();
    AtFork(nullptr, nullptr, OnFork);
  }

  static FastRandomNumberGenerator& engine() noexcept { return engine_; }

 private:
  static thread_local FastRandomNumberGenerator engine_;

  static void OnFork() noexcept { Seed(); }

  static void Seed() noexcept {
    std::random_device random_device;
    std::seed_seq seed_seq{random_device(), random_device(), random_device(),
                           random_device()};
    engine_.seed(seed_seq);
  }
};

thread_local FastRandomNumberGenerator TlsRandomNumberGenerator::engine_{};
}  // namespace

//--------------------------------------------------------------------------------------------------
// GetRandomNumberGenerator
//--------------------------------------------------------------------------------------------------
FastRandomNumberGenerator& GetRandomNumberGenerator() noexcept {
  static thread_local TlsRandomNumberGenerator random_number_generator{};
  return TlsRandomNumberGenerator::engine();
}

//--------------------------------------------------------------------------------------------------
// GenerateRandomDuration
//--------------------------------------------------------------------------------------------------
std::chrono::nanoseconds GenerateRandomDuration(
    std::chrono::nanoseconds a, std::chrono::nanoseconds b) noexcept {
  assert(a <= b);
  std::uniform_int_distribution<std::chrono::nanoseconds::rep> distribution{
      a.count(), b.count()};
  return std::chrono::nanoseconds{distribution(GetRandomNumberGenerator())};
}
}  // namespace lightstep

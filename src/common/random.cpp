#include "common/random.h"

#include <cassert>

#include "common/platform/fork.h"
#include "lightstep/randutils.h"

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
  TlsRandomNumberGenerator() { AtFork(nullptr, nullptr, OnFork); }

  static std::mt19937_64& engine() { return base_generator_.engine(); }

 private:
  static thread_local lightstep::randutils::mt19937_64_rng base_generator_;

  static void OnFork() { base_generator_.seed(); }
};

thread_local lightstep::randutils::mt19937_64_rng
    TlsRandomNumberGenerator::base_generator_{};
}  // namespace

//--------------------------------------------------------------------------------------------------
// GetRandomNumberGenerator
//--------------------------------------------------------------------------------------------------
std::mt19937_64& GetRandomNumberGenerator() {
  static TlsRandomNumberGenerator random_number_generator;
  return TlsRandomNumberGenerator::engine();
}

//--------------------------------------------------------------------------------------------------
// GenerateId
//--------------------------------------------------------------------------------------------------
uint64_t GenerateId() { return GetRandomNumberGenerator()(); }

//--------------------------------------------------------------------------------------------------
// GenerateRandomDuration
//--------------------------------------------------------------------------------------------------
std::chrono::nanoseconds GenerateRandomDuration(std::chrono::nanoseconds a,
                                                std::chrono::nanoseconds b) {
  assert(a <= b);
  std::uniform_int_distribution<std::chrono::nanoseconds::rep> distribution{
      a.count(), b.count()};
  return std::chrono::nanoseconds{distribution(GetRandomNumberGenerator())};
}
}  // namespace lightstep

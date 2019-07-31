#include "common/random.h"

#include <cassert>

#include "lightstep/randutils.h"

#include <pthread.h>

namespace lightstep {
//------------------------------------------------------------------------------
// TlsRandomNumberGenerator
//------------------------------------------------------------------------------
// Wraps a thread_local random number generator, but adds a fork handler so that
// the generator will be correctly seeded after forking.
//
// See https://stackoverflow.com/q/51882689/4447365 and
//     https://github.com/opentracing-contrib/nginx-opentracing/issues/52
#if 0
namespace {
class TlsRandomNumberGenerator {
 public:
  using BaseGenerator = randutils::random_generator<FastRandomNumberGenerator>;

  TlsRandomNumberGenerator() noexcept { ::pthread_atfork(nullptr, nullptr, OnFork); }

  static FastRandomNumberGenerator& engine() noexcept {
    return base_generator_.engine();
  }

 private:
  static thread_local BaseGenerator base_generator_;

  static void OnFork() noexcept { base_generator_.seed(); }
};

thread_local TlsRandomNumberGenerator::BaseGenerator
    TlsRandomNumberGenerator::base_generator_{};
}  // namespace
#endif
thread_local TlsRandomNumberGenerator::BaseGenerator
    TlsRandomNumberGenerator::base_generator_{};
TlsRandomNumberGenerator::TlsRandomNumberGenerator() noexcept { ::pthread_atfork(nullptr, nullptr, OnFork); }
thread_local TlsRandomNumberGenerator RandomNumberGenerator{};

//--------------------------------------------------------------------------------------------------
// GetRandomNumberGenerator
//--------------------------------------------------------------------------------------------------
FastRandomNumberGenerator& GetRandomNumberGenerator() noexcept {
  static thread_local TlsRandomNumberGenerator random_number_generator{};
  return TlsRandomNumberGenerator::engine();
}

/* thread_local FastRandomNumberGenerator& RandomNumberGenerator = */
/*     GetRandomNumberGenerator(); */

//--------------------------------------------------------------------------------------------------
// GenerateRandomDuration
//--------------------------------------------------------------------------------------------------
std::chrono::nanoseconds GenerateRandomDuration(std::chrono::nanoseconds a,
                                                std::chrono::nanoseconds b) noexcept {
  assert(a <= b);
  std::uniform_int_distribution<std::chrono::nanoseconds::rep> distribution{
      a.count(), b.count()};
  return std::chrono::nanoseconds{distribution(GetRandomNumberGenerator())};
}
}  // namespace lightstep

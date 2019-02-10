#include "common/random.h"

#include <random>

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
namespace {
class TlsRandomNumberGenerator {
 public:
  TlsRandomNumberGenerator() { ::pthread_atfork(nullptr, nullptr, OnFork); }

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
static std::mt19937_64& GetRandomNumberGenerator() {
  static TlsRandomNumberGenerator random_number_generator;
  return TlsRandomNumberGenerator::engine();
}

//--------------------------------------------------------------------------------------------------
// GenerateId
//--------------------------------------------------------------------------------------------------
uint64_t GenerateId() { return GetRandomNumberGenerator()(); }
}  // namespace lightstep

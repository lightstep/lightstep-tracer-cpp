#include "common/fast_random_number_generator.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// operator()()
//--------------------------------------------------------------------------------------------------
uint64_t FastRandomNumberGenerator::operator()() noexcept {
  // Uses the xorshift128p random number algorithm described in
  // https://en.wikipedia.org/wiki/Xorshift
  auto& state_a = state_[0];
  auto& state_b = state_[1];
  auto t = state_a;
  auto s = state_b;
  state_a = s;
  t ^= t << 23;        // a
  t ^= t >> 17;        // b
  t ^= s ^ (s >> 26);  // c
  state_b = t;
  return t + s;
}
}  // namespace lightstep

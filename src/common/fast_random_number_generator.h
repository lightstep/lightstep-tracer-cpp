#pragma once

#include <array>
#include <cstdint>
#include <limits>

namespace lightstep {
class FastRandomNumberGenerator {
 public:
  using result_type = uint64_t;

  FastRandomNumberGenerator() noexcept = default;

  template <class SeedSequence>
  FastRandomNumberGenerator(SeedSequence& seed_sequence) noexcept {
    seed(seed_sequence);
  }

  uint64_t operator()() noexcept;

  template <class SeedSequence>
  void seed(SeedSequence& seed_sequence) noexcept {
    seed_sequence.generate(
        reinterpret_cast<uint32_t*>(state_.data()),
        reinterpret_cast<uint32_t*>(state_.data() + state_.size()));
  }

  uint64_t min() const noexcept { return 0; }

  uint64_t max() const noexcept { return std::numeric_limits<uint64_t>::max(); }

 private:
  std::array<uint64_t, 2> state_{};
};
}  // namespace lightstep

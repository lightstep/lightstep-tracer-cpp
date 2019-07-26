#pragma once

#include <array>
#include <cstdint>

namespace lightstep {
class FastRandomNumberGenerator {
 public:
  uint64_t operator()() noexcept;

  template <class SeedSequence>
  void seed(SeedSequence& seed_sequence) noexcept {
    seed_sequence.generate(
        reinterpret_cast<uint32_t*>(state_.data()),
        reinterpret_cast<uint32_t*>(state_.data() + state_.size()));
  }

 private:
  std::array<uint64_t, 2> state_{};
};
}  // namespace lightstep

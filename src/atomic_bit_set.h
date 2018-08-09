#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace lightstep {
class AtomicBitSet {
 public:
  using BlockType = uint64_t;

  explicit AtomicBitSet(size_t size);

  bool Test(int bit_index) const noexcept;

  bool Reset(int bit_index) noexcept;

  bool Set(int bit_index) noexcept;

 private:

  static constexpr int bits_per_block =
      static_cast<int>(std::numeric_limits<BlockType>::digits);

  std::vector<std::atomic<BlockType>> blocks_;

  int ComputeBlockIndex(int bit_index) const noexcept;

  int ComputeBitOffSet(int bit_index) const noexcept;

};
}  // namespace lightstep

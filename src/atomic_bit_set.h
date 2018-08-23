#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace lightstep {
// An atomic bit set of dynamic size.
//
// Modeled after Folly's AtomicBitSet. See
// https://github.com/facebook/folly/blob/cd1bdc912603c0358ba733d379a74ae90ab3a437/folly/AtomicBitSet.h
class AtomicBitSet {
 public:
  using BlockType = uint64_t;

  explicit AtomicBitSet(size_t size);

  // Gets the current value of the bit at `bit_index`.
  bool Test(int bit_index) const noexcept;

  // Sets the bit at `bit_index` to false and returns the current value.
  bool Reset(int bit_index) noexcept;

  // Sets the bit at `bit_index` to true and returns the current value.
  bool Set(int bit_index) noexcept;

 private:
  static constexpr int bits_per_block =
      static_cast<int>(std::numeric_limits<BlockType>::digits);

  std::vector<std::atomic<BlockType>> blocks_;

  int ComputeBlockIndex(int bit_index) const noexcept;

  int ComputeBitOffSet(int bit_index) const noexcept;
};
}  // namespace lightstep

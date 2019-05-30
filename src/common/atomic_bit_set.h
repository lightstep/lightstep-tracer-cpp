#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace lightstep {
/**
 * An atomic bit set of dynamic size.
 *
 * Modeled after Folly's AtomicBitSet. See
 * https://github.com/facebook/folly/blob/cd1bdc912603c0358ba733d379a74ae90ab3a437/folly/AtomicBitSet.h
 */
class AtomicBitSet {
 public:
  using BlockType = uint64_t;

  explicit AtomicBitSet(size_t size);

  /**
   * Tests whether a given bit is set.
   * @param bit_index identifies the bit to test.
   * @return the value of the specified bit.
   */
  bool Test(int bit_index) const noexcept;

  /**
   * Resets a given bit to 0.
   * @param bit_index identifies the bit to reset.
   * @return the previous value of the given bit.
   */
  bool Reset(int bit_index) noexcept;

  /**
   * Sets a given bit to 1.
   * @param bit_index identifies the bit to set.
   * @return the previous value of the given bit.
   */
  bool Set(int bit_index) noexcept;

  /**
   * Reset all bits to 0.
   */
  void Clear() noexcept;

 private:
  static constexpr int bits_per_block =
      static_cast<int>(std::numeric_limits<BlockType>::digits);

  std::vector<std::atomic<BlockType>> blocks_;

  int ComputeBlockIndex(int bit_index) const noexcept;

  int ComputeBitOffSet(int bit_index) const noexcept;
};
}  // namespace lightstep

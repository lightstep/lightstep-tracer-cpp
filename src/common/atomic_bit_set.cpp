#include "common/atomic_bit_set.h"

#include <algorithm>

namespace lightstep {
static constexpr AtomicBitSet::BlockType one = 1;

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
AtomicBitSet::AtomicBitSet(size_t size)
    : blocks_(size / bits_per_block +
              static_cast<size_t>(size % bits_per_block > 0)) {}

//------------------------------------------------------------------------------
// ComputeBlockIndex
//------------------------------------------------------------------------------
int AtomicBitSet::ComputeBlockIndex(int bit_index) const noexcept {
  return bit_index / bits_per_block;
}

//------------------------------------------------------------------------------
// ComputeBitOffSet
//------------------------------------------------------------------------------
int AtomicBitSet::ComputeBitOffSet(int bit_index) const noexcept {
  return bit_index % bits_per_block;
}

//------------------------------------------------------------------------------
// Test
//------------------------------------------------------------------------------
bool AtomicBitSet::Test(int bit_index) const noexcept {
  auto mask = one << ComputeBitOffSet(bit_index);
  return static_cast<bool>(blocks_[ComputeBlockIndex(bit_index)].load() & mask);
}

//------------------------------------------------------------------------------
// Reset
//------------------------------------------------------------------------------
bool AtomicBitSet::Reset(int bit_index) noexcept {
  auto mask = one << ComputeBitOffSet(bit_index);
  return static_cast<bool>(
      blocks_[ComputeBlockIndex(bit_index)].fetch_and(~mask) & mask);
}

//------------------------------------------------------------------------------
// Set
//------------------------------------------------------------------------------
bool AtomicBitSet::Set(int bit_index) noexcept {
  auto mask = one << ComputeBitOffSet(bit_index);
  return static_cast<bool>(
      blocks_[ComputeBlockIndex(bit_index)].fetch_or(mask) & mask);
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void AtomicBitSet::Clear() noexcept {
  std::fill(blocks_.begin(), blocks_.end(), 0);
}
}  // namespace lightstep

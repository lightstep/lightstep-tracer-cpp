#include "atomic_bit_set.h"

namespace lightstep {
static constexpr AtomicBitSet::BlockType one = 1;

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
AtomicBitSet::AtomicBitSet(size_t size)
    : blocks_(size / bits_per_block + (size % bits_per_block > 0)) {}

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
  return blocks_[ComputeBlockIndex(bit_index)].load() & mask;
}

//------------------------------------------------------------------------------
// Reset
//------------------------------------------------------------------------------
void AtomicBitSet::Reset(int bit_index) noexcept {
  auto mask = one << ComputeBitOffSet(bit_index);
  blocks_[ComputeBlockIndex(bit_index)].fetch_and(~mask);
}

//------------------------------------------------------------------------------
// Set
//------------------------------------------------------------------------------
void AtomicBitSet::Set(int bit_index) noexcept {
  auto mask = one << ComputeBitOffSet(bit_index);
  blocks_[ComputeBlockIndex(bit_index)].fetch_or(mask);
}
}  // namespace lightstep

#include "common/chained_stream.h"

#include <cassert>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ChainedStream::ChainedStream() noexcept : current_block_{&head_} {}

//--------------------------------------------------------------------------------------------------
// CloseOutput
//--------------------------------------------------------------------------------------------------
void ChainedStream::CloseOutput() noexcept {
  output_closed_ = true;
  current_block_ = &head_;
}

//--------------------------------------------------------------------------------------------------
// Next
//--------------------------------------------------------------------------------------------------
bool ChainedStream::Next(void** data, int* size) {
  assert(!output_closed_);

  if (current_block_position_ < BlockSize) {
    *size = BlockSize - current_block_position_;
    *data = static_cast<void*>(current_block_->data.data() +
                               current_block_position_);
    num_bytes_written_ += *size;
    current_block_position_ = BlockSize;
    current_block_->size = BlockSize;
    return true;
  }
  current_block_->next.reset(new Block{});
  current_block_ = current_block_->next.get();
  current_block_->size = BlockSize;
  current_block_position_ = BlockSize;
  *size = BlockSize;
  num_bytes_written_ += *size;
  *data = static_cast<void*>(current_block_->data.data());
  ++num_blocks_;
  return true;
}

//--------------------------------------------------------------------------------------------------
// BackUp
//--------------------------------------------------------------------------------------------------
void ChainedStream::BackUp(int count) {
  assert(!output_closed_);

  num_bytes_written_ -= count;
  current_block_position_ -= count;
  current_block_->size -= count;
}

//--------------------------------------------------------------------------------------------------
// segment_num_fragments
//--------------------------------------------------------------------------------------------------
int ChainedStream::segment_num_fragments() const noexcept {
  assert(output_closed_);

  if (num_bytes_written_ == 0) {
    return 0;
  }
  return num_blocks_ - fragment_index_;
}

//--------------------------------------------------------------------------------------------------
// SegmentForEachFragment
//--------------------------------------------------------------------------------------------------
bool ChainedStream::SegmentForEachFragment(Callback callback) const noexcept {
  assert(output_closed_);
  assert(fragment_index_ >= 0 && fragment_index_ <= num_blocks_);

  if (num_blocks_ == 0) {
    return true;
  }

  auto block = current_block_;

  auto block_size = block->size;
  assert(block_size >= fragment_position_);
  if (!callback(static_cast<void*>(const_cast<char*>(block->data.data()) +
                                   fragment_position_),
                block_size - fragment_position_)) {
    return false;
  }

  block = block->next.get();
  while (block != nullptr) {
    if (!callback(static_cast<void*>(const_cast<char*>(block->data.data())),
                  block->size)) {
      return false;
    }
    block = block->next.get();
  }
  return true;
}

//--------------------------------------------------------------------------------------------------
// SegmentClear
//--------------------------------------------------------------------------------------------------
void ChainedStream::SegmentClear() noexcept {
  assert(output_closed_);

  num_blocks_ = 0;
  num_bytes_written_ = 0;
  fragment_index_ = 0;
  fragment_position_ = 0;
  current_block_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
// SegmentSeek
//--------------------------------------------------------------------------------------------------
void ChainedStream::SegmentSeek(int relative_fragment_index,
                                int position) noexcept {
  assert(output_closed_);
  assert(fragment_index_ + relative_fragment_index <= num_blocks_);
  if (relative_fragment_index == 0) {
    fragment_position_ += position;
    assert(fragment_position_ <= current_block_->size);
    return;
  }
  for (int i = 0; i < relative_fragment_index; ++i) {
    current_block_ = current_block_->next.get();
  }
  fragment_index_ += relative_fragment_index;
  fragment_position_ = position;
  assert(fragment_position_ <= current_block_->size);
}
}  // namespace lightstep

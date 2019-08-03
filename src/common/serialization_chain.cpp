#include "common/serialization_chain.h"

#include <algorithm>
#include <cassert>

#include "common/utility.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace lightstep {
static const char* LineTerminator = "\r\n";

//--------------------------------------------------------------------------------------------------
// WriteChunkHeader
//--------------------------------------------------------------------------------------------------
static int WriteChunkHeader(char* data, size_t data_length,
                            size_t chunk_size) noexcept {
  (void)data_length;
  auto chunk_size_str = Uint32ToHex(static_cast<uint32_t>(chunk_size), data);
  assert(chunk_size_str.size() + 2 <= data_length);
  assert(!chunk_size_str.empty());
  auto iter = data + chunk_size_str.size();
  *iter++ = '\r';
  *iter++ = '\n';
  return static_cast<int>(std::distance(data, iter));
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SerializationChain:: SerializationChain(BlockAllocator& allocator) noexcept
  : allocator_{allocator}, current_block_{&head_} {
    head_.next = nullptr;
    head_.size = FirstBlockSize;
    head_.max_size = head_.size;
}

static BlockAllocator HeapBlockAllocator{sizeof(SerializationChain), 0};

SerializationChain::SerializationChain() noexcept
    : SerializationChain{HeapBlockAllocator} {}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
SerializationChain::~SerializationChain() noexcept {
  FreeBlocks(head_.next);
}

//--------------------------------------------------------------------------------------------------
// AddFraming
//--------------------------------------------------------------------------------------------------
void SerializationChain::AddFraming() noexcept {
  auto protobuf_header_size =
      ComputeLengthDelimitedHeaderSerializationSize<ReportRequestSpansField>(
          num_bytes_written_);
  (void)WriteChunkHeader;
  header_size_ = WriteChunkHeader(
      header_.data(), header_.size(),
      static_cast<size_t>(num_bytes_written_) + protobuf_header_size);
  assert(header_size_ > 0);

  // Serialize the spans key field and length
  DirectCodedOutputStream stream{reinterpret_cast<google::protobuf::uint8*>(
      header_.data() + header_size_)};
  WriteKeyLength<ReportRequestSpansField>(
      stream, static_cast<size_t>(num_bytes_written_));
  header_size_ += protobuf_header_size;

  // Prepare the data structure to act as a FragmentInputStream.
  current_block_ = &head_;
}

//--------------------------------------------------------------------------------------------------
// Next
//--------------------------------------------------------------------------------------------------
bool SerializationChain::Next(void** data, int* size) {
  if (current_block_position_ < current_block_->max_size) {
    *size = current_block_->max_size - current_block_position_;
    *data = static_cast<void*>(current_block_->data() +
                               current_block_position_);
    num_bytes_written_ += *size;
    current_block_position_ = current_block_->max_size;
    current_block_->size = current_block_->max_size;
    return true;
  }
  current_block_->next = AllocateNewBlock();
  current_block_ = current_block_->next;
  current_block_position_ = current_block_->size;
  *size = current_block_->size;
  num_bytes_written_ += *size;
  *data = static_cast<void*>(current_block_->data());
  ++num_blocks_;
  return true;
}

//--------------------------------------------------------------------------------------------------
// BackUp
//--------------------------------------------------------------------------------------------------
void SerializationChain::BackUp(int count) {
  num_bytes_written_ -= count;
  current_block_position_ -= count;
  current_block_->size -= count;
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int SerializationChain::num_fragments() const noexcept {
  if (num_bytes_written_ == 0) {
    return 0;
  }
  return num_blocks_ + 2 - fragment_index_;
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SerializationChain::ForEachFragment(Callback callback) const noexcept {
  assert(fragment_index_ >= 0 && fragment_index_ <= num_blocks_ + 1);

  if (num_blocks_ == 0) {
    return true;
  }

  // header
  if (fragment_index_ == 0) {
    assert(fragment_position_ < header_size_);
    if (!callback(static_cast<void*>(const_cast<char*>(header_.data()) +
                                     fragment_position_),
                  header_size_ - fragment_position_)) {
      return false;
    }
  }

  // data
  auto block = current_block_;
  if (fragment_index_ > 0 && fragment_index_ <= num_blocks_) {
    auto block_size = block->size;
    assert(block_size >= fragment_position_);
    if (!callback(static_cast<void*>(const_cast<char*>(block->data()) +
                                     fragment_position_),
                  block_size - fragment_position_)) {
      return false;
    }
    block = block->next;
  }
  while (block != nullptr) {
    if (!callback(static_cast<void*>(const_cast<char*>(block->data())),
                  block->size)) {
      return false;
    }
    block = block->next;
  }

  // chunk trailer
  if (fragment_index_ == num_blocks_ + 1) {
    assert(fragment_position_ < 2);
    return callback(static_cast<void*>(const_cast<char*>(LineTerminator) +
                                       fragment_position_),
                    2 - fragment_position_);
  }
  return callback(static_cast<void*>(const_cast<char*>(LineTerminator)), 2);
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void SerializationChain::Clear() noexcept {
  num_blocks_ = 0;
  num_bytes_written_ = 0;
  fragment_index_ = 0;
  fragment_position_ = 0;
  current_block_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SerializationChain::Seek(int fragment_index, int position) noexcept {
  if (fragment_index == 0) {
    fragment_position_ += position;
    return;
  }
  auto prev_fragment_index = fragment_index_;
  fragment_index_ += fragment_index;
  if (fragment_index_ == num_blocks_ + 1) {
    assert(position < 2);
    fragment_position_ = position;
    current_block_ = nullptr;
    return;
  }

  int prev_block_index = std::max(prev_fragment_index - 1, 0);
  int block_index = fragment_index_ - 1;
  for (int i = prev_block_index; i < block_index; ++i) {
    current_block_ = current_block_->next;
  }
  fragment_position_ = position;
}

//--------------------------------------------------------------------------------------------------
// AllocateNewBlock
//--------------------------------------------------------------------------------------------------
SerializationChain::Block* SerializationChain::AllocateNewBlock() {
  auto result = static_cast<Block*>(allocator_.allocate());
  result->next = nullptr;
  result->size = static_cast<int>(allocator_.block_size() - sizeof(Block));
  result->max_size = result->size;
  return result;
}

//--------------------------------------------------------------------------------------------------
// FreeBlocks
//--------------------------------------------------------------------------------------------------
void SerializationChain::FreeBlocks(Block* block) noexcept {
  if (block == nullptr) {
    return;
  }
  auto next_block = block->next;
  allocator_.deallocate(static_cast<void*>(block));
  FreeBlocks(next_block);
}
}  // namespace lightstep

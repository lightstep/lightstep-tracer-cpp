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
  std::array<char, Num32BitHexDigits> buffer;
  auto chunk_size_str =
      Uint32ToHex(static_cast<uint32_t>(chunk_size), buffer.data());
  assert(chunk_size_str.size() + 2 <= data_length);
  assert(!chunk_size_str.empty());
  auto first = std::find_if(chunk_size_str.begin(), chunk_size_str.end() - 1,
                            [](char c) { return c != '0'; });
  auto iter = std::copy(first, chunk_size_str.end(), data);
  *iter++ = '\r';
  *iter++ = '\n';
  return static_cast<int>(std::distance(data, iter));
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SerializationChain::SerializationChain() noexcept : current_block_{&head_} {}

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
    if (!callback(static_cast<void*>(const_cast<char*>(block->data.data()) +
                                     fragment_position_),
                  block_size - fragment_position_)) {
      return false;
    }
    block = block->next.get();
  }
  while (block != nullptr) {
    if (!callback(static_cast<void*>(const_cast<char*>(block->data.data())),
                  block->size)) {
      return false;
    }
    block = block->next.get();
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
    current_block_ = current_block_->next.get();
  }
  fragment_position_ = position;
}
}  // namespace lightstep

#include "common/chunk_circular_buffer.h"
#include "common/bipart_memory_stream.h"
#include "common/utility.h"

#include <cassert>
#include <cstdio>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ChunkCircularBuffer::ChunkCircularBuffer(size_t max_bytes)
    : ready_flags_{max_bytes + 1}, buffer_{max_bytes} {}

//--------------------------------------------------------------------------------------------------
// Add
//--------------------------------------------------------------------------------------------------
bool ChunkCircularBuffer::Add(Serializer serializer, size_t size) noexcept {
  assert(size > 0);
  static const auto line_terminator = "\r\n";
  std::array<char, Num64BitHexDigits + 1> chunk_buffer;  // Note: We add space
                                                         // for an additional
                                                         // byte to account for
                                                         // snprintf's null
                                                         // terminator.
  auto num_chunk_size_chars =
      std::snprintf(chunk_buffer.data(), chunk_buffer.size(), "%llX",
                    static_cast<unsigned long long>(size));
  assert(num_chunk_size_chars > 0);
  auto placement =
      buffer_.Reserve(static_cast<size_t>(num_chunk_size_chars) + 4 + size);
  if (placement.data1 == nullptr) {
    return false;
  }
  BipartMemoryOutputStream zero_copy_stream{placement.data1, placement.size1,
                                            placement.data2, placement.size2};
  google::protobuf::io::CodedOutputStream output_stream{&zero_copy_stream};
  output_stream.WriteRaw(static_cast<const void*>(chunk_buffer.data()),
                         num_chunk_size_chars);
  output_stream.WriteRaw(static_cast<const void*>(line_terminator), 2);
  serializer(output_stream);
  output_stream.WriteRaw(static_cast<const void*>(line_terminator), 2);
  auto prev_ready_state =
      ready_flags_.Set(static_cast<int>(placement.data1 - buffer_.data()));
  (void)prev_ready_state;
  assert(!prev_ready_state);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Allot
//--------------------------------------------------------------------------------------------------
int ChunkCircularBuffer::Allot() noexcept {
  int num_chunks_allotted = 0;
  while (true) {
    auto placement = buffer_.PeekFromPosition(num_bytes_allotted_);
    if (placement.size1 == 0) {
      return num_chunks_allotted;
    }
    if (!ready_flags_.Reset(
            static_cast<int>(placement.data1 - buffer_.data()))) {
      return num_chunks_allotted;
    }
    BipartMemoryInputStream stream{placement.data1, placement.size1,
                                   placement.data2, placement.size2};
    size_t chunk_size;
    auto was_successful = ReadChunkHeader(stream, chunk_size);
    (void)was_successful;
    assert(was_successful);
    num_bytes_allotted_ += stream.ByteCount() + chunk_size + 2;
    assert(stream.Skip(static_cast<int>(chunk_size + 2)));
    ++num_chunks_allotted;
  }
}

//--------------------------------------------------------------------------------------------------
// Consume
//--------------------------------------------------------------------------------------------------
void ChunkCircularBuffer::Consume(size_t num_bytes) noexcept {
  assert(num_bytes <= num_bytes_allotted_);
  num_bytes_allotted_ -= num_bytes;
  buffer_.Consume(num_bytes);
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void ChunkCircularBuffer::Clear() noexcept {
  ready_flags_.Clear();
  buffer_.Consume(buffer_.size());
  num_bytes_allotted_ = 0;
}

//--------------------------------------------------------------------------------------------------
// FindChunk
//--------------------------------------------------------------------------------------------------
std::tuple<CircularBufferConstPlacement, int> ChunkCircularBuffer::FindChunk(
    const char* start, const char* ptr) const noexcept {
  auto position = buffer_.ComputePosition(start);
  auto index = buffer_.ComputePosition(ptr);
  assert(position <= num_bytes_allotted_);
  assert(index <= num_bytes_allotted_);
  assert(position <= index);
  int chunk_count = 0;
  while (true) {
    auto placement = buffer_.PeekFromPosition(position);
    BipartMemoryInputStream stream{placement.data1, placement.size1,
                                   placement.data2, placement.size2};
    size_t chunk_size;
    auto was_successful = ReadChunkHeader(stream, chunk_size);
    (void)was_successful;
    assert(was_successful);
    auto total_size = stream.ByteCount() + chunk_size + 2;
    if (index < position + total_size) {
      return std::make_tuple(buffer_.Peek(position, total_size), chunk_count);
    }
    ++chunk_count;
    position += total_size;
  }
}
}  // namespace lightstep

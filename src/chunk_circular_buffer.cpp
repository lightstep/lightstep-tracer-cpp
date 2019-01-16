#include "chunk_circular_buffer.h"

#include "bipart_memory_stream.h"
#include "utility.h"

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
bool ChunkCircularBuffer::Add(Serializer serializer, size_t size,
                              void* context) noexcept {
  assert(size > 0);
  static const auto line_terminator = "\r\n";
  std::array<char, 17> chunk_buffer;
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
  serializer(output_stream, size, context);
  output_stream.WriteRaw(static_cast<const void*>(line_terminator), 2);
  auto prev_ready_state =
      ready_flags_.Set(static_cast<int>(placement.data1 - buffer_.data()));
  assert(!prev_ready_state);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Allot
//--------------------------------------------------------------------------------------------------
void ChunkCircularBuffer::Allot() noexcept {
  while (1) {
    auto placement = buffer_.PeekFromPosition(num_bytes_allotted_);
    if (placement.size1 == 0) {
      return;
    }
    if (!ready_flags_.Reset(
            static_cast<int>(placement.data1 - buffer_.data()))) {
      return;
    }
    BipartMemoryInputStream stream{placement.data1, placement.size1,
                                   placement.data2, placement.size2};
    size_t chunk_size;
    auto was_successful = ReadChunkHeader(stream, chunk_size);
    assert(was_successful);
    num_bytes_allotted_ += stream.ByteCount() + chunk_size + 2;
    assert(stream.Skip(static_cast<int>(chunk_size + 2)));
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
}  // namespace lightstep

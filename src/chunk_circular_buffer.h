#pragma once

#include "atomic_bit_set.h"
#include "circular_buffer.h"

#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
class ChunkCircularBuffer {
 public:
  using Serializer = void(google::protobuf::io::CodedOutputStream& stream,
                          size_t size, void* context);

  explicit ChunkCircularBuffer(size_t max_bytes);

  bool Add(Serializer serializer, size_t size, void* context) noexcept;

  void Allot() noexcept;

  void Consume(size_t num_bytes) noexcept;

  CircularBufferConstPlacement allotment() const noexcept {
    return buffer_.Peek(0, num_bytes_allotted_);
  }

  bool empty() const noexcept { return buffer_.empty(); }

  size_t max_size() const noexcept { return buffer_.max_size(); }

  size_t num_bytes_allotted() const noexcept { return num_bytes_allotted_; }

 private:
  AtomicBitSet ready_flags_;
  CircularBuffer buffer_;
  size_t num_bytes_allotted_{0};
};
}  // namespace lightstep

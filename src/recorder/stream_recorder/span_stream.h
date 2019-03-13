#pragma once

#include "common/chunk_circular_buffer.h"
#include "network/fragment_input_stream.h"

namespace lightstep {
class SpanStream final : public FragmentInputStream {
 public:
  explicit SpanStream(ChunkCircularBuffer& span_buffer) noexcept;

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(FunctionRef<FragmentInputStream::Callback> callback)
      const noexcept override;

 private:
  ChunkCircularBuffer& span_buffer_;
};
}  // namespace lightstep

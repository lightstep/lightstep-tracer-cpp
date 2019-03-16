#pragma once

#include "common/chunk_circular_buffer.h"
#include "network/fragment_input_stream.h"

namespace lightstep {
/**
 * Manages the stream of data comming from the circular buffer of completed
 * spans.
 */
class SpanStream final : public FragmentInputStream {
 public:
  explicit SpanStream(ChunkCircularBuffer& span_buffer) noexcept;

  void Allot() noexcept;

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  ChunkCircularBuffer& span_buffer_;
  CircularBufferConstPlacement allotment_;
  const char* stream_position_{nullptr};
};
}  // namespace lightstep

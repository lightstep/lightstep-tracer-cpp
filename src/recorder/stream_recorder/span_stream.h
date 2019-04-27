#pragma once

#include <vector>

#include "common/chunk_circular_buffer.h"
#include "network/fragment_array_input_stream.h"
#include "recorder/stream_recorder/fragment_span_input_stream.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
/**
 * Manages the stream of data comming from the circular buffer of completed
 * spans.
 */
class SpanStream final : public FragmentInputStream {
 public:
  SpanStream(ChunkCircularBuffer& span_buffer, StreamRecorderMetrics& metrics,
             int max_connections);

  /**
   * Allots spans from the ChunkCircularBuffer to stream to satellites.
   */
  void Allot() noexcept;

  /**
   * Removes the marker of the remnant of a span so that its bytes can be
   * consumed.
   * @param remnant the span remnant to remove.
   */
  void RemoveSpanRemnant(const char* remnant) noexcept;

  /**
   * Pops the span remnant left by the write.
   * @param remnant where to output the last remnant.
   */
  void PopSpanRemnant(FragmentSpanInputStream& remnant) noexcept;

  /**
   * Consumes bytes no longer needed from the associated ChunkCircularBuffer.
   */
  void Consume() noexcept;

  StreamRecorderMetrics& metrics() const noexcept { return metrics_; }

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  ChunkCircularBuffer& span_buffer_;
  StreamRecorderMetrics& metrics_;
  CircularBufferConstPlacement allotment_;
  const char* stream_position_{nullptr};
  FragmentSpanInputStream span_remnant_;
  std::vector<const char*> span_remnants_;

  void SetPositionAfter(const CircularBufferConstPlacement& placement) noexcept;
};
}  // namespace lightstep

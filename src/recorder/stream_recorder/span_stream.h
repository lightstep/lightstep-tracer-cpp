#pragma once

#include "common/chained_stream.h"
#include "common/circular_buffer.h"
#include "recorder/metrics_tracker.h"

namespace lightstep {
/**
 * Manages the stream of data comming from the circular buffer of completed
 * spans.
 */
class SpanStream final : public FragmentInputStream {
 public:
  SpanStream(CircularBuffer<ChainedStream>& span_buffer,
             MetricsTracker& metrics) noexcept;

  /**
   * Allots spans from the associated circular buffer to stream to satellites.
   */
  void Allot() noexcept;

  /**
   * Returns and removes the last partially written span.
   * @return the last partially written span
   */
  std::unique_ptr<ChainedStream> ConsumeRemnant() noexcept;

  /**
   * @return the associagted MetricsTracker
   */
  MetricsTracker& metrics() const noexcept { return metrics_; }

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  CircularBuffer<ChainedStream>& span_buffer_;
  MetricsTracker& metrics_;
  CircularBufferRange<const AtomicUniquePtr<ChainedStream>> allotment_;
  std::unique_ptr<ChainedStream> remnant_;
};
}  // namespace lightstep

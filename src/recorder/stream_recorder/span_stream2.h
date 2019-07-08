#pragma once

#include "common/circular_buffer2.h"
#include "common/serialization_chain.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
class SpanStream2 final : public FragmentInputStream {
 public:
  SpanStream2(CircularBuffer2<SerializationChain>& span_buffer,
              StreamRecorderMetrics& metrics) noexcept;

  void Allot() noexcept;

  std::unique_ptr<SerializationChain> ConsumeRemnant() noexcept;

  StreamRecorderMetrics& metrics() const noexcept { return metrics_; }

  // FragmentInputStream
  int num_fragments() const noexcept override;

  bool ForEachFragment(Callback callback) const noexcept override;

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override;

 private:
  CircularBuffer2<SerializationChain>& span_buffer_;
  StreamRecorderMetrics& metrics_;
  CircularBufferRange<const AtomicUniquePtr<SerializationChain>> allotment_;
  std::unique_ptr<SerializationChain> remnant_;
};
}  // namespace lightstep

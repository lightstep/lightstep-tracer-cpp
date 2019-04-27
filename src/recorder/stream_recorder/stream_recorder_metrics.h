#pragma once

#include <atomic>

#include <lightstep/metrics_observer.h>

namespace lightstep {
/**
 * Manages the metrics associated with a StreamRecorder.
 */
class StreamRecorderMetrics {
 public:
  explicit StreamRecorderMetrics(MetricsObserver& metrics_observer) noexcept;

  void OnSpansDropped(int num_spans) noexcept;

  void OnSpansSent(int num_spans) noexcept;

  int ConsumeDroppedSpans() noexcept;

  void UnconsumeDroppedSpans(int num_spans) noexcept;

  int num_dropped_spans() const noexcept { return num_dropped_spans_; }

 private:
  MetricsObserver& metrics_observer_;
  std::atomic<int> num_dropped_spans_{0};
};
}  // namespace lightstep

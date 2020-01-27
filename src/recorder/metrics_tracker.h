#pragma once

#include <atomic>

#include <lightstep/metrics_observer.h>

namespace lightstep {
/**
 * Manages the metrics associated with a StreamRecorder.
 */
class MetricsTracker {
 public:
  explicit MetricsTracker(MetricsObserver& metrics_observer) noexcept;

  /**
   * Record dropped spans.
   * @param num_spans the number of spans dropped.
   */
  inline void OnSpansDropped(int num_spans) noexcept {
    metrics_observer_.OnSpansDropped(num_spans);
    num_dropped_spans_ += num_spans;
  }

  /**
   * Record spans sent.
   * @param num_spans the number of spans sent.
   */
  void OnSpansSent(int num_spans) noexcept;

  /**
   * Record flushes.
   */
  void OnFlush() noexcept;

  /**
   * Return then clear the dropped span counter.
   * @return the dropped span count.
   */
  int ConsumeDroppedSpans() noexcept;

  /**
   * Add spans back to the dropped span counter.
   * @param num_spans the number of dropped spans to add back.
   */
  void UnconsumeDroppedSpans(int num_spans) noexcept;

  /**
   * @return the dropped span count.
   */
  int num_dropped_spans() const noexcept { return num_dropped_spans_; }

 private:
  MetricsObserver& metrics_observer_;
  std::atomic<int> num_dropped_spans_{0};
};
}  // namespace lightstep

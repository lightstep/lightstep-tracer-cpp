#pragma once

#include <atomic>

#include "lightstep/metrics_observer.h"

namespace lightstep {
/**
 * Recorder for dropped spans
 */
class SpanDropCounter : public MetricsObserver {
 public:
  /**
   * @return the number of dropped spans recorded
   */
  int num_dropped_spans() const noexcept { return num_dropped_spans_; }

  // MetricsObserver
  void OnSpansDropped(int num_spans) noexcept override {
    num_dropped_spans_ += num_spans;
  }

 private:
  std::atomic<int> num_dropped_spans_{0};
};
}  // namespace lightstep

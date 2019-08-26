#pragma once

#include <atomic>

#include "lightstep/metrics_observer.h"

namespace lightstep {
struct CountingMetricsObserver : MetricsObserver {
  // MetricsObserver
  void OnSpansSent(int num_spans) noexcept override {
    num_spans_sent += num_spans;
  }

  void OnSpansDropped(int num_spans) noexcept override {
    num_spans_dropped += num_spans;
  }

  void OnFlush() noexcept override { ++num_flushes; }

  std::atomic<int> num_flushes{0};
  std::atomic<int> num_spans_sent{0};
  std::atomic<int> num_spans_dropped{0};
};
}  // namespace lightstep

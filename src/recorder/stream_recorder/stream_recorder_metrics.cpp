#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
StreamRecorderMetrics::StreamRecorderMetrics(
    MetricsObserver& metrics_observer) noexcept
    : metrics_observer_{metrics_observer} {}

//--------------------------------------------------------------------------------------------------
// OnSpansDropped
//--------------------------------------------------------------------------------------------------
void StreamRecorderMetrics::OnSpansDropped(int num_spans) noexcept {
  try {
    metrics_observer_.OnSpansDropped(num_spans);
  } catch (...) {
  }
  num_dropped_spans_ += num_spans;
}

//--------------------------------------------------------------------------------------------------
// OnSpansSent
//--------------------------------------------------------------------------------------------------
void StreamRecorderMetrics::OnSpansSent(int num_spans) noexcept {
  try {
    metrics_observer_.OnSpansSent(num_spans);
  } catch (...) {
  }
}

//--------------------------------------------------------------------------------------------------
// OnFlush
//--------------------------------------------------------------------------------------------------
void StreamRecorderMetrics::OnFlush() noexcept {
  try {
    metrics_observer_.OnFlush();
  } catch (...) {
  }
}

//--------------------------------------------------------------------------------------------------
// ConsumeDroppedSpans
//--------------------------------------------------------------------------------------------------
int StreamRecorderMetrics::ConsumeDroppedSpans() noexcept {
  return num_dropped_spans_.exchange(0);
}

//--------------------------------------------------------------------------------------------------
// UnconsumeDroppedSpans
//--------------------------------------------------------------------------------------------------
void StreamRecorderMetrics::UnconsumeDroppedSpans(int num_spans) noexcept {
  num_dropped_spans_ += num_spans;
}
}  // namespace lightstep

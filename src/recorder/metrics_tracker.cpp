#include "recorder/metrics_tracker.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
MetricsTracker::MetricsTracker(MetricsObserver& metrics_observer) noexcept
    : metrics_observer_{metrics_observer} {}

//--------------------------------------------------------------------------------------------------
// OnSpansSent
//--------------------------------------------------------------------------------------------------
void MetricsTracker::OnSpansSent(int num_spans) noexcept {
  metrics_observer_.OnSpansSent(num_spans);
}

//--------------------------------------------------------------------------------------------------
// OnFlush
//--------------------------------------------------------------------------------------------------
void MetricsTracker::OnFlush() noexcept { metrics_observer_.OnFlush(); }

//--------------------------------------------------------------------------------------------------
// ConsumeDroppedSpans
//--------------------------------------------------------------------------------------------------
int MetricsTracker::ConsumeDroppedSpans() noexcept {
  return num_dropped_spans_.exchange(0);
}

//--------------------------------------------------------------------------------------------------
// UnconsumeDroppedSpans
//--------------------------------------------------------------------------------------------------
void MetricsTracker::UnconsumeDroppedSpans(int num_spans) noexcept {
  num_dropped_spans_ += num_spans;
}
}  // namespace lightstep

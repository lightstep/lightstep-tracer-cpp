#pragma once

#include <atomic>

namespace lightstep {
/**
 * Manages the metrics associated with a StreamRecorder.
 */
struct StreamRecorderMetrics {
  std::atomic<int> num_dropped_spans{0};
};
}  // namespace lightstep

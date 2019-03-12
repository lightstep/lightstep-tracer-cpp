#pragma once

#include <atomic>

namespace lightstep {
struct StreamRecorderMetrics {
  std::atomic<int> num_dropped_spans{0};
};
}  // namespace lightstep

#pragma once

#include <utility>
#include <vector>

#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
class EmbeddedMetricsMessage {
 public:
  EmbeddedMetricsMessage();

  void set_num_dropped_spans(int num_dropped_spans) const noexcept;

  int num_dropped_spans() const noexcept;

  std::pair<void*, int> MakeFragment();

 private:
  collector::InternalMetrics message_;
  collector::MetricsSample& dropped_spans_count_;
  std::vector<char> buffer_;
};
}  // namespace lightstep

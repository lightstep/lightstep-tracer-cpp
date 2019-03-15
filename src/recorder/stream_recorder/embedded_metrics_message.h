#pragma once

#include <utility>
#include <vector>

#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
/**
 * Manages the metrics message sent in a ReportRequest.
 */
class EmbeddedMetricsMessage {
 public:
  EmbeddedMetricsMessage();

  /**
   * Sets the number of dropped spans to send in the message.
   * @param num_dropped_spans the number of dropped spans to set.
   */
  void set_num_dropped_spans(int num_dropped_spans) const noexcept;

  /**
   * @return the number of dropped spans in the message.
   */
  int num_dropped_spans() const noexcept;

  /**
   * @return a fragment for the serialized message.
   */
  std::pair<void*, int> MakeFragment();

 private:
  collector::InternalMetrics message_;
  collector::MetricsSample& dropped_spans_count_;
  std::vector<char> buffer_;
};
}  // namespace lightstep

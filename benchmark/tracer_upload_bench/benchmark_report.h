#pragma once

#include <chrono>
#include <iosfwd>

namespace lightstep {
struct BenchmarkReport {
  std::chrono::microseconds duration;
  int num_spans_generated;
  int num_dropped_spans;
  int approx_span_size;
};

std::ostream& operator<<(std::ostream& out, const BenchmarkReport& report);
}  // namespace lightstep

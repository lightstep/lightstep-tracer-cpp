#include "benchmark_report.h"

#include <iostream>

namespace lightstep {
std::ostream& operator<<(std::ostream& out, const BenchmarkReport& report) {
  std::cout << "Approx Span size (bytes): " << report.approx_span_size << "\n";
  std::cout << "Total spans: " << report.num_spans_generated << "\n";
  std::cout << "Dropped spans: " << report.num_dropped_spans << "\n";

  auto dropped_span_percent = 100.0 *
                              static_cast<double>(report.num_dropped_spans) /
                              report.num_spans_generated;
  std::cout << "Dropped spans (%): " << dropped_span_percent << "\n";
  auto num_spans_sent = report.num_spans_generated - report.num_dropped_spans;
  auto upload_rate =
      1e6 * static_cast<double>(num_spans_sent) / report.duration.count();
  std::cout << "Upload rate (spans/sec): " << upload_rate << "\n";
  auto upload_rate_bytes = 1e6 * static_cast<double>(num_spans_sent) *
                           report.approx_span_size / report.duration.count();
  std::cout << "Approx upload (bytes/sec): " << upload_rate_bytes << "\n";
  return out;
}
}  // namespace lightstep

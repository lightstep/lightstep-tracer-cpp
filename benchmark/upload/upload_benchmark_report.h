#pragma once

#include <iosfwd>

#include "configuration-proto/upload_benchmark_configuration.pb.h"

namespace lightstep {
struct UploadBenchmarkReport {
  upload_benchmark_configuration::UploadBenchmarkConfiguration configuration;
  size_t total_spans;
  size_t num_spans_received;
  size_t num_spans_dropped;
  size_t num_reported_dropped_spans;
  double elapse;
  double spans_per_second;
};

std::ostream& operator<<(std::ostream& out,
                         const UploadBenchmarkReport& report);
}  // namespace lightstep

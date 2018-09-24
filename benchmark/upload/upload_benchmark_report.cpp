#include "upload_benchmark_report.h"

#include <iostream>

namespace lightstep {
std::ostream& operator<<(std::ostream& out,
                         const UploadBenchmarkReport& report) {
  if (report.configuration.tracer_configuration().use_stream_recorder()) {
    out << "tracer type: stream\n";
    out << "message_buffer_size: "
        << report.configuration.tracer_configuration().message_buffer_size()
        << "\n";
  } else {
    out << "tracer type: rpc\n";
    out << "max_buffered_spans: "
        << report.configuration.tracer_configuration().max_buffered_spans()
        << "\n";
  }
  out << "num threads: " << report.configuration.num_threads() << "\n";
  out << "num spans generated: " << report.configuration.num_spans() << "\n";
  out << "max_spans_per_second: " << report.configuration.max_spans_per_second()
      << "\n";
  out << "num spans dropped: " << report.num_spans_dropped << "\n";
  out << "num reported dropped spans: " << report.num_reported_dropped_spans
      << "\n";
  out << "elapse (seconds): " << report.elapse << "\n";
  out << "spans_per_second: " << report.spans_per_second << "\n";
  return out;
}
}  // namespace lightstep

#include "lightstep/tracer.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "benchmark_report.h"
#include "span.h"
#include "utility.h"
using namespace lightstep;

int main(int argc, char* argv[]) try {
  if (argc != 2) {
    std::cout << "Usage: span-upload-bench <benchmark_configuration>\n";
    return 1;
  }
  auto config = ParseConfiguration(argv[1]);
  std::shared_ptr<opentracing::Tracer> tracer;
  SpanDropCounter* span_drop_counter;
  std::tie(tracer, span_drop_counter) = MakeTracer(config);
  std::this_thread::sleep_for(std::chrono::milliseconds{
      static_cast<int>(1000 * config.startup_delay())});
  auto t1 = std::chrono::steady_clock::now();
  GenerateSpans(*tracer, config);
  auto t2 = std::chrono::steady_clock::now();
  BenchmarkReport report;
  report.num_spans_generated =
      config.num_spans_per_thread() * config.num_threads();
  report.num_dropped_spans = span_drop_counter->num_dropped_spans();
  report.duration =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
  report.approx_span_size = static_cast<int>(ComputeSpanSize(config));
  std::cout << report;
  return 0;
} catch (const std::exception& e) {
  std::cerr << e.what() << "\n";
  return -1;
}

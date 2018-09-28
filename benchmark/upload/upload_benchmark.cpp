#include "upload_benchmark.h"

#include <chrono>
#include <thread>

#include "../src/lightstep_span.h"

namespace lightstep {
//------------------------------------------------------------------------------
// MakeReport
//------------------------------------------------------------------------------
static UploadBenchmarkReport MakeReport(
    const configuration_proto::UploadBenchmarkConfiguration& configuration,
    const std::vector<uint64_t>& sent_span_ids, const DummySatellite* satellite,
    std::chrono::system_clock::duration elapse) {
  auto received_span_ids = satellite->span_ids();
  UploadBenchmarkReport result;
  result.configuration = configuration;
  result.total_spans = sent_span_ids.size();
  result.num_spans_received = received_span_ids.size();
  result.num_spans_dropped = result.total_spans - result.num_spans_received;
  result.num_reported_dropped_spans = satellite->num_dropped_spans();
  result.elapse =
      1.0e-6 *
      std::chrono::duration_cast<std::chrono::microseconds>(elapse).count();
  result.spans_per_second = result.total_spans / result.elapse;

  // Sanity check
  std::set<uint64_t> span_ids{sent_span_ids.begin(), sent_span_ids.end()};
  for (auto id : received_span_ids) {
    if (span_ids.find(id) == span_ids.end()) {
      std::cerr << "Upload Error: " << id << " not sent\n";
      std::terminate();
    }
  }
  return result;
}

//------------------------------------------------------------------------------
// GenerateSpans
//------------------------------------------------------------------------------
static void GenerateSpans(opentracing::Tracer& tracer, int num_spans,
                          std::chrono::system_clock::duration min_span_elapse,
                          uint64_t* output) {
  auto start_timestamp = std::chrono::system_clock::now();
  for (int i = 0; i < num_spans; ++i) {
    auto span = tracer.StartSpan("abc");
    auto lightstep_span = static_cast<LightStepSpan*>(span.get());
    *output++ =
        static_cast<const LightStepSpanContext&>(lightstep_span->context())
            .span_id();
    std::this_thread::sleep_until(start_timestamp + min_span_elapse * i);
  }
}

//------------------------------------------------------------------------------
// RunUploadBenchmark
//------------------------------------------------------------------------------
UploadBenchmarkReport RunUploadBenchmark(
    const configuration_proto::UploadBenchmarkConfiguration& configuration,
    DummySatellite* satellite, std::shared_ptr<opentracing::Tracer>& tracer) {
  satellite->Reserve(configuration.num_spans());
  std::vector<uint64_t> span_ids(configuration.num_spans());
  std::vector<std::thread> threads(configuration.num_threads());

  auto seconds_per_span = 1.0 / configuration.max_spans_per_second();
  auto min_span_elapse =
      std::chrono::duration_cast<std::chrono::system_clock::duration>(
          std::chrono::microseconds{
              static_cast<size_t>(seconds_per_span * 1e6)});

  auto spans_per_thread =
      configuration.num_spans() / configuration.num_threads();
  auto remainder = configuration.num_spans() -
                   spans_per_thread * configuration.num_threads();
  auto span_id_output = span_ids.data();
  auto start_time = std::chrono::system_clock::now();
  for (int i = 0; i < static_cast<int>(configuration.num_threads()); ++i) {
    auto num_spans_for_this_thread =
        spans_per_thread + (i < static_cast<int>(remainder));
    threads[i] =
        std::thread{&GenerateSpans, std::ref(*tracer),
                    num_spans_for_this_thread, min_span_elapse, span_id_output};
    span_id_output += num_spans_for_this_thread;
  }
  for (auto& thread : threads) {
    thread.join();
  }
  auto finish_time = std::chrono::system_clock::now();

  // Make sure the tracer's destructor runs so that the stream dummy satellite
  // will receive an EOF.
  tracer->Close();
  tracer.reset();
  satellite->Close();
  auto elapse = finish_time - start_time;

  return MakeReport(configuration, span_ids, satellite, elapse);
}
}  // namespace lightstep

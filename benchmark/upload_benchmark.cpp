#include "../src/lightstep_span.h"
#include "grpc_dummy_satellite.h"
#include "stream_dummy_satellite.h"

#include "configuration-proto/upload_benchmark_configuration.pb.h"

#include <google/protobuf/util/json_util.h>
#include <lightstep/tracer.h>

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <set>
#include <thread>
#include <vector>
using namespace lightstep;

//------------------------------------------------------------------------------
// BenchmarkOptions
//------------------------------------------------------------------------------
struct BenchmarkOptions {
  size_t num_spans;
  size_t num_threads;
  size_t max_spans_per_second;
};

//------------------------------------------------------------------------------
// UploadReport
//------------------------------------------------------------------------------
struct UploadReport {
  size_t total_spans;
  size_t num_spans_received;
  size_t num_spans_dropped;
  double elapse;
  double spans_per_second;
};

//------------------------------------------------------------------------------
// MakeReport
//------------------------------------------------------------------------------
static UploadReport MakeReport(const std::vector<uint64_t>& sent_span_ids,
                               const std::vector<uint64_t>& received_span_ids,
                               std::chrono::system_clock::duration elapse) {
  UploadReport result;
  result.total_spans = sent_span_ids.size();
  result.num_spans_received = received_span_ids.size();
  result.num_spans_dropped = result.total_spans - result.num_spans_received;
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
static UploadReport RunUploadBenchmark(
    const BenchmarkOptions& options, DummySatellite& satellite,
    std::shared_ptr<opentracing::Tracer>& tracer) {
  satellite.Reserve(options.num_spans);
  std::vector<uint64_t> span_ids(options.num_spans);
  std::vector<std::thread> threads(options.num_threads);

  auto seconds_per_span = 1.0 / options.max_spans_per_second;
  auto min_span_elapse =
      std::chrono::duration_cast<std::chrono::system_clock::duration>(
          std::chrono::microseconds{
              static_cast<size_t>(seconds_per_span * 1e6)});

  auto spans_per_thread = options.num_spans / options.num_threads;
  auto remainder = options.num_spans - spans_per_thread * options.num_threads;
  auto span_id_output = span_ids.data();
  auto start_time = std::chrono::system_clock::now();
  for (int i = 0; i < static_cast<int>(options.num_threads); ++i) {
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
  satellite.Close();
  auto elapse = finish_time - start_time;

  return MakeReport(span_ids, satellite.span_ids(), elapse);
}

//------------------------------------------------------------------------------
// SetupGrpcTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> SetupGrpcTracer(
    std::unique_ptr<DummySatellite>& satellite, size_t max_buffered_spans) {
  LightStepTracerOptions options{};
  options.component_name = "test";
  options.access_token = "abc123";
  options.collector_host = "localhost";
  options.collector_port = 9000;
  options.collector_plaintext = true;
  options.max_buffered_spans = max_buffered_spans;
  auto grpc_satellite = new GrpcDummySatellite{"localhost:9000"};
  satellite.reset(grpc_satellite);
  grpc_satellite->Run();
  return MakeLightStepTracer(std::move(options));
}

//------------------------------------------------------------------------------
// SetupStreamTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> SetupStreamTracer(
    std::unique_ptr<DummySatellite>& satellite, size_t max_buffered_spans) {
  LightStepTracerOptions options{};
  options.component_name = "test";
  options.access_token = "abc123";
  options.collector_host = "127.0.0.1";
  options.collector_port = 9000;
  options.collector_plaintext = true;
  options.use_stream_recorder = true;

  // max_buffered_spans * sizeof(collector::Span) provides a lower bound on the
  // amount of memory used for buffering by the grpc tracer since it disregards
  // any heap allocation.
  options.message_buffer_size = max_buffered_spans * sizeof(collector::Span);

  auto stream_satellite = new StreamDummySatellite{"127.0.0.1", 9000};
  satellite.reset(stream_satellite);
  return MakeLightStepTracer(std::move(options));
}

//------------------------------------------------------------------------------
// ParseBenchmarkConfiguration
//------------------------------------------------------------------------------
static lightstep::upload_benchmark_configuration::UploadBenchmarkConfiguration
ParseBenchmarkConfiguration(const char* filename) {
  std::ifstream in{filename};
  if (!in.good()) {
    std::cerr << "Failed to open " << filename << ": " << std::strerror(errno)
              << "\n";
    std::terminate();
  }
  std::string configuration{std::istreambuf_iterator<char>{in},
                            std::istreambuf_iterator<char>{}};
  if (!in.good()) {
    std::cerr << "Iostream error: " << std::strerror(errno) << "\n";
    std::terminate();
  }
  lightstep::upload_benchmark_configuration::UploadBenchmarkConfiguration
      result;
  auto parse_result =
      google::protobuf::util::JsonStringToMessage(configuration, &result);
  if (!parse_result.ok()) {
    std::cerr << "Failed to parse benchmark configuration: "
              << parse_result.ToString() << "\n";
    std::terminate();
  }
  return result;
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cout << "Usage: upload_test <rpc|stream> <max_buffered_spans> "
                 "<num_threads> <num_spans> <max_spans_per_second>\n";
    return 0;
  }
  std::string tracer_type{argv[1]};
  BenchmarkOptions options;
  auto max_buffered_spans = static_cast<size_t>(std::atoi(argv[2]));
  options.num_threads = static_cast<size_t>(std::atoi(argv[3]));
  options.num_spans = static_cast<size_t>(std::atoi(argv[4]));
  options.max_spans_per_second = static_cast<size_t>(std::atoi(argv[5]));

  std::unique_ptr<DummySatellite> satellite;
  std::shared_ptr<opentracing::Tracer> tracer;
  if (tracer_type == "rpc") {
    tracer = SetupGrpcTracer(satellite, max_buffered_spans);
  } else if (tracer_type == "stream") {
    tracer = SetupStreamTracer(satellite, max_buffered_spans);
  } else {
    std::cerr << "Unknown tracer type: " << tracer_type << "\n";
    return -1;
  }

  auto report = RunUploadBenchmark(options, *satellite, tracer);
  std::cout << "tracer type: " << tracer_type << "\n";
  std::cout << "max_buffered_spans: " << max_buffered_spans << "\n";
  std::cout << "num threads: " << options.num_threads << "\n";
  std::cout << "num spans generated: " << options.num_spans << "\n";
  std::cout << "max_spans_per_second: " << options.max_spans_per_second << "\n";
  std::cout << "num spans dropped: " << report.num_spans_dropped << "\n";
  std::cout << "elapse (seconds): " << report.elapse << "\n";
  std::cout << "spans_per_second: " << report.spans_per_second << "\n";

  return 0;
}

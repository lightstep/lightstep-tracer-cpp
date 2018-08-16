#include "grpc_dummy_satellite.h"
#include "stream_dummy_satellite.h"
#include "../src/lightstep_span.h"

#include <lightstep/tracer.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <set>
#include <exception>
using namespace lightstep;

//------------------------------------------------------------------------------
// UploadReport
//------------------------------------------------------------------------------
struct UploadReport {
 size_t total_spans;
 size_t num_spans_received;
 size_t num_spans_dropped;
};

//------------------------------------------------------------------------------
// MakeReport
//------------------------------------------------------------------------------
static UploadReport MakeReport(const std::vector<uint64_t>& sent_span_ids,
    const std::vector<uint64_t>& received_span_ids) {
  UploadReport result;
  result.total_spans = sent_span_ids.size();
  result.num_spans_received = received_span_ids.size();
  result.num_spans_dropped = result.total_spans - result.num_spans_received;

  // Sanity check
  /* std::set<uint64_t> span_ids{sent_span_ids.begin(), sent_span_ids.end()}; */
  /* for (auto id : received_span_ids) { */
  /*   if (span_ids.find(id) == span_ids.end()) { */
  /*     std::cerr << id << " not sent\n"; */
  /*     std::terminate(); */
  /*   } */
  /* } */
  return result;
}

//------------------------------------------------------------------------------
// RunBenchmark1
//------------------------------------------------------------------------------
static UploadReport RunBenchmark1(DummySatellite& satellite,
                   opentracing::Tracer& tracer,
                   std::chrono::steady_clock::duration duration) {
  /* const int num_spans = 5000000; */
  const int num_spans = 1000000;
  satellite.Reserve(num_spans);
  std::vector<uint64_t> span_ids;
  span_ids.reserve(num_spans);

  const auto interval = duration / num_spans;
  for (int i=0; i<num_spans; ++i) {
    auto span = tracer.StartSpan("abc");
    auto lightstep_span = static_cast<LightStepSpan*>(span.get());
    span_ids.push_back(
        static_cast<const LightStepSpanContext&>(lightstep_span->context())
            .span_id());
    /* std::this_thread::sleep_for(interval); */
  }
  tracer.Close();

  return MakeReport(span_ids, satellite.span_ids());
}

//------------------------------------------------------------------------------
// SetupGrpcTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> SetupGrpcTracer(
    std::unique_ptr<DummySatellite>& satellite) {
  LightStepTracerOptions options{};
  options.component_name = "test";
  options.access_token = "abc123";
  options.collector_host = "localhost";
  options.collector_port = 9000;
  options.collector_plaintext = true;
  auto grpc_satellite = new GrpcDummySatellite{"localhost:9000"};
  satellite.reset(grpc_satellite);
  grpc_satellite->Run();
  return MakeLightStepTracer(std::move(options));
}


//------------------------------------------------------------------------------
// SetupStreammingTracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> SetupStreamingTracer(
    std::unique_ptr<DummySatellite>& satellite) {
  LightStepTracerOptions options{};
  options.component_name = "test";
  options.access_token = "abc123";
  options.collector_host = "127.0.0.1";
  options.collector_port = 9000;
  options.collector_plaintext = true;
  options.use_streaming_recorder = true;
  auto stream_satellite = new StreamDummySatellite{"127.0.0.1", 9000};
  satellite.reset(stream_satellite);
  return MakeLightStepTracer(std::move(options));
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: upload_test <rpc|stream>\n";
    return 0;
  }
  (void)argv;

  /* const char* server_address = "localhost:9000"; */

  std::unique_ptr<DummySatellite> satellite;
  /* auto tracer = SetupGrpcTracer(satellite); */
  auto tracer = SetupStreamingTracer(satellite);
  /* auto report1 = RunBenchmark1(*satellite, *tracer, std::chrono::minutes{1}); */
  auto report1 = RunBenchmark1(*satellite, *tracer, std::chrono::seconds{1});
  std::cout << report1.total_spans << " / " << report1.num_spans_dropped << "\n";
  /* satellite = [] { */
  /*  LightStepTracerOptions options; */ 
  /* }(); */

  return 0;
}

#include "../src/lightstep_span.h"
#include "grpc_dummy_satellite.h"
#include "stream_dummy_satellite.h"
#include "tracer.h"
#include "upload_benchmark.h"

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
// ParseBenchmarkConfiguration
//------------------------------------------------------------------------------
static lightstep::configuration_proto::UploadBenchmarkConfiguration
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
  lightstep::configuration_proto::UploadBenchmarkConfiguration result;
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

  configuration_proto::UploadBenchmarkConfiguration configuration;

  configuration.set_num_threads(static_cast<uint32_t>(std::atoi(argv[3])));
  configuration.set_num_spans(static_cast<uint32_t>(std::atoi(argv[4])));
  configuration.set_max_spans_per_second(
      static_cast<uint32_t>(std::atoi(argv[5])));

  // Set defaults for connectivity
  auto& tracer_configuration = *configuration.mutable_tracer_configuration();
  auto max_buffered_spans = static_cast<uint32_t>(std::atoi(argv[2]));
  if (tracer_type == "rpc") {
    tracer_configuration.set_max_buffered_spans(max_buffered_spans);
  } else if (tracer_type == "stream") {
    // Use a lower bound on the amount of memory that the RPC recorder's buffer
    // will take.
    tracer_configuration.set_message_buffer_size(max_buffered_spans *
                                                 sizeof(collector::Span));
    tracer_configuration.set_use_stream_recorder(true);
  } else {
    std::cerr << "unknown tracer type: " << tracer_type << "\n";
    std::terminate();
  }
  tracer_configuration.set_access_token("abc123");
  tracer_configuration.set_collector_host("127.0.0.1");
  tracer_configuration.set_collector_plaintext(true);
  tracer_configuration.set_collector_port(9000);

  auto satellite = MakeDummySatellite(tracer_configuration);
  auto tracer = MakeTracer(tracer_configuration);
  auto report = RunUploadBenchmark(configuration, satellite.get(), tracer);
  std::cout << report;
  return 0;
}

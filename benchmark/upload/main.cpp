#include "../../src/lightstep_span.h"
#include "../../test/dummy_satellite/dummy_satellite.h"
#include "upload_benchmark.h"

#include "configuration-proto/upload_benchmark_configuration.pb.h"

#include <gflags/gflags.h>
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

// Workaround for compiler warning with gflags.
//
// See https://github.com/gflags/gflags/issues/261
namespace GFLAGS_NAMESPACE {
extern template FlagRegisterer::FlagRegisterer(const char*, const char*,
                                               const char*, int*, int*);
extern template FlagRegisterer::FlagRegisterer(const char*, const char*,
                                               const char*, std::string*,
                                               std::string*);
}  // namespace GFLAGS_NAMESPACE

DECLARE_string(config);
DECLARE_string(recorder_type);
DECLARE_int32(max_buffered_spans);
DECLARE_int32(num_threads);
DECLARE_int32(num_spans);
DECLARE_int32(max_spans_per_second);

DEFINE_string(
    config, "",
    "File with a json configuration specifying the benchmark options");

DEFINE_string(recorder_type, "",
              "The type of recorder to use for uploading spans. Can be either "
              "rpc or stream");

DEFINE_int32(max_buffered_spans, 0,
             "The maximum number of spans to buffer in the recorder");

DEFINE_int32(num_threads, 0,
             "The number of threads to use when generating spans");

DEFINE_int32(num_spans, 0, "The number of spans to generate");

DEFINE_int32(max_spans_per_second, 0,
             "The maximum number of spans to generate per second");

//------------------------------------------------------------------------------
// ParseBenchmarkConfiguration
//------------------------------------------------------------------------------
static void ParseBenchmarkConfiguration(
    const char* filename,
    configuration_proto::UploadBenchmarkConfiguration& configuration) {
  std::ifstream in{filename};
  if (!in.good()) {
    std::cerr << "Failed to open " << filename << ": " << std::strerror(errno)
              << "\n";
    std::terminate();
  }
  std::string json{std::istreambuf_iterator<char>{in},
                   std::istreambuf_iterator<char>{}};
  if (!in.good()) {
    std::cerr << "Iostream error: " << std::strerror(errno) << "\n";
    std::terminate();
  }
  auto parse_result =
      google::protobuf::util::JsonStringToMessage(json, &configuration);
  if (!parse_result.ok()) {
    std::cerr << "Failed to parse benchmark configuration: "
              << parse_result.ToString() << "\n";
    std::terminate();
  }
}

//------------------------------------------------------------------------------
// HasRequiredOptions
//------------------------------------------------------------------------------
static bool HasRequiredOptions() {
  if (!FLAGS_config.empty()) {
    return true;
  }
  return !FLAGS_recorder_type.empty() && FLAGS_max_buffered_spans > 0 &&
         FLAGS_num_threads > 0 && FLAGS_num_spans > 0 &&
         FLAGS_max_spans_per_second > 0;
}

//------------------------------------------------------------------------------
// MakeConfiguration
//------------------------------------------------------------------------------
static configuration_proto::UploadBenchmarkConfiguration MakeConfiguration(
    int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (!HasRequiredOptions()) {
    GFLAGS_NAMESPACE::ShowUsageWithFlags(argv[0]);
    std::exit(-1);
  }
  configuration_proto::UploadBenchmarkConfiguration result;

  if (!FLAGS_config.empty()) {
    ParseBenchmarkConfiguration(FLAGS_config.c_str(), result);
  }

  if (FLAGS_num_threads > 0) {
    result.set_num_threads(FLAGS_num_threads);
  }

  if (FLAGS_num_spans > 0) {
    result.set_num_spans(FLAGS_num_spans);
  }

  if (FLAGS_max_spans_per_second > 0) {
    result.set_max_spans_per_second(FLAGS_max_spans_per_second);
  }

  auto& tracer_configuration = *result.mutable_tracer_configuration();
  if (!FLAGS_recorder_type.empty()) {
    if (FLAGS_recorder_type == "rpc") {
      tracer_configuration.set_use_stream_recorder(false);
    } else if (FLAGS_recorder_type == "stream") {
      tracer_configuration.set_use_stream_recorder(true);
    } else {
      std::cerr << "Invalid recorder type \"" << FLAGS_recorder_type << "\"\n";
      std::terminate();
    }
  }
  if (FLAGS_config.empty()) {
    result.set_use_dummy_satellite(true);
    tracer_configuration.set_access_token("abc123");
    tracer_configuration.set_collector_host("127.0.0.1");
    tracer_configuration.set_collector_plaintext(true);
    tracer_configuration.set_collector_port(9000);
  }
  return result;
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  auto configuration = MakeConfiguration(argc, argv);
  std::unique_ptr<DummySatellite> satellite;
  if (configuration.use_dummy_satellite()) {
    satellite = MakeDummySatellite(configuration.tracer_configuration());
  }
  auto report = RunUploadBenchmark(configuration, satellite.get());
  std::cout << report;
  return 0;
}

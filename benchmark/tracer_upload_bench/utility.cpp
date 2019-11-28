#include "utility.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "tracer/json_options.h"
#include "tracer/lightstep_tracer_factory.h"

#include "google/protobuf/util/json_util.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ReadFile
//--------------------------------------------------------------------------------------------------
static std::string ReadFile(const char* filename) {
  std::ifstream in{filename};
  if (!in.good()) {
    std::ostringstream oss;
    oss << "Failed to open " << filename << ": " << std::strerror(errno);
    throw std::runtime_error{oss.str()};
  }
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  return std::string{std::istreambuf_iterator<char>{in},
                     std::istreambuf_iterator<char>{}};
}

//--------------------------------------------------------------------------------------------------
// MakeTracerOptions
//--------------------------------------------------------------------------------------------------
static LightStepTracerOptions MakeTracerOptions(
    const tracer_configuration::TracerConfiguration& config) {
  std::string tracer_config_json;
  auto convert_result =
      google::protobuf::util::MessageToJsonString(config, &tracer_config_json);
  if (!convert_result.ok()) {
    std::ostringstream oss;
    oss << "Failed to construct tracer configuration: "
        << convert_result.ToString();
    throw std::runtime_error{oss.str()};
  }
  std::string error_message;
  auto result_maybe =
      MakeTracerOptions(tracer_config_json.c_str(), error_message);
  if (!result_maybe) {
    throw std::runtime_error{error_message};
  }
  return std::move(*result_maybe);
}

//--------------------------------------------------------------------------------------------------
// ParseConfiguration
//--------------------------------------------------------------------------------------------------
tracer_upload_bench::Configuration ParseConfiguration(const char* filename) {
  auto config_json = ReadFile(filename);
  tracer_upload_bench::Configuration result;
  auto parse_result =
      google::protobuf::util::JsonStringToMessage(config_json, &result);
  if (!parse_result.ok()) {
    std::ostringstream oss;
    oss << "Failed to parse configuration json: " << parse_result.ToString();
    throw std::runtime_error{oss.str()};
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
std::tuple<std::shared_ptr<LightStepTracer>, SpanDropCounter*> MakeTracer(
    const tracer_upload_bench::Configuration& config) {
  auto tracer_options = MakeTracerOptions(config.tracer_configuration());
  auto span_drop_counter = new SpanDropCounter{};
  tracer_options.metrics_observer.reset(span_drop_counter);
  return std::make_tuple(MakeLightStepTracer(std::move(tracer_options)),
                         span_drop_counter);
  ;
}
}  // namespace lightstep

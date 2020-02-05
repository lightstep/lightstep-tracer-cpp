#include "tracer/json_options.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <google/protobuf/util/json_util.h>
#include <opentracing/tracer_factory.h>
#include "lightstep-tracer-configuration/tracer_configuration.pb.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GetSatelliteEndpoints
//--------------------------------------------------------------------------------------------------
static std::vector<std::pair<std::string, uint16_t>> GetSatelliteEndpoints(
    const tracer_configuration::TracerConfiguration& tracer_configuration) {
  std::vector<std::pair<std::string, uint16_t>> result;
  const auto& satellite_endpoints = tracer_configuration.satellite_endpoints();
  result.reserve(satellite_endpoints.size());
  for (auto& endpoint : satellite_endpoints) {
    auto port = endpoint.port();
    if (port == 0) {
      throw std::runtime_error{"endpoint port outside of range"};
    }
    if (port > std::numeric_limits<uint16_t>::max()) {
      throw std::runtime_error{"endpoint port outside of range"};
    }
    result.emplace_back(endpoint.host(), static_cast<uint16_t>(port));
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// GetPropagationMode
//--------------------------------------------------------------------------------------------------
static PropagationMode GetPropagationMode(opentracing::string_view s) {
  if (s == "b3") {
    return PropagationMode::b3;
  }
  if (s == "lightstep") {
    return PropagationMode::lightstep;
  }
  if (s == "envoy") {
    return PropagationMode::envoy;
  }
  if (s == "trace_context") {
    return PropagationMode::trace_context;
  }
  std::ostringstream oss;
  oss << "invalid propagation mode " << s;
  throw std::runtime_error{oss.str()};
}

//--------------------------------------------------------------------------------------------------
// MakeTracerOptions
//--------------------------------------------------------------------------------------------------
opentracing::expected<LightStepTracerOptions> MakeTracerOptions(
    const char* configuration, std::string& error_message) try {
  tracer_configuration::TracerConfiguration tracer_configuration;
  auto parse_result = google::protobuf::util::JsonStringToMessage(
      configuration, &tracer_configuration);
  if (!parse_result.ok()) {
    error_message = parse_result.ToString();
    return opentracing::make_unexpected(opentracing::configuration_parse_error);
  }
  LightStepTracerOptions options;
  options.component_name = tracer_configuration.component_name();
  options.access_token = tracer_configuration.access_token();

  if (!tracer_configuration.collector_host().empty()) {
    options.collector_host = tracer_configuration.collector_host();
  }
  if (tracer_configuration.collector_port() != 0) {
    options.collector_port = tracer_configuration.collector_port();
  }
  options.collector_plaintext = tracer_configuration.collector_plaintext();
  options.use_single_key_propagation =
      tracer_configuration.use_single_key_propagation();

  if (tracer_configuration.max_buffered_spans() != 0) {
    options.max_buffered_spans = tracer_configuration.max_buffered_spans();
  }

  if (tracer_configuration.reporting_period() != 0) {
    options.reporting_period =
        std::chrono::microseconds{tracer_configuration.reporting_period()};
  }

  if (tracer_configuration.report_timeout() != 0) {
    options.report_timeout =
        std::chrono::microseconds{tracer_configuration.report_timeout()};
  }

  options.use_stream_recorder = tracer_configuration.use_stream_recorder();

  options.satellite_endpoints = GetSatelliteEndpoints(tracer_configuration);

  options.verbose = tracer_configuration.verbose();

  options.propagation_modes.reserve(
      tracer_configuration.propagation_modes().size());
  for (auto& propagation_mode : tracer_configuration.propagation_modes()) {
    options.propagation_modes.push_back(GetPropagationMode(propagation_mode));
  }

  return options;
} catch (const std::exception& e) {
  std::cerr << "Invalid options: " << e.what() << "\n";
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::invalid_argument));
}
}  // namespace lightstep

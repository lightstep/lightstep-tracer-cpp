#include "lightstep_tracer_factory.h"
#include <google/protobuf/util/json_util.h>
#include <lightstep/tracer.h>

namespace lightstep {
//------------------------------------------------------------------------------
// MakeTracerOptions
//------------------------------------------------------------------------------
LightStepTracerOptions MakeTracerOptions(
    const tracer_configuration::TracerConfiguration& configuration) {
  LightStepTracerOptions options;
  options.component_name = configuration.component_name();
  options.access_token = configuration.access_token();

  if (!configuration.collector_host().empty()) {
    options.collector_host = configuration.collector_host();
  }
  if (configuration.collector_port() != 0) {
    options.collector_port = configuration.collector_port();
  }
  options.collector_plaintext = configuration.collector_plaintext();
  options.use_single_key_propagation =
      configuration.use_single_key_propagation();

  options.use_stream_recorder = configuration.use_stream_recorder();

  if (configuration.max_buffered_spans() != 0) {
    options.max_buffered_spans = configuration.max_buffered_spans();
  }

  if (configuration.message_buffer_size() != 0) {
    options.message_buffer_size = configuration.message_buffer_size();
  }

  if (configuration.reporting_period() != 0) {
    options.reporting_period =
        std::chrono::microseconds{configuration.reporting_period()};
  }

  if (configuration.report_timeout() != 0) {
    options.report_timeout =
        std::chrono::microseconds{configuration.report_timeout()};
  }
  return options;
}

//------------------------------------------------------------------------------
// MakeTracer
//------------------------------------------------------------------------------
opentracing::expected<std::shared_ptr<opentracing::Tracer>>
LightStepTracerFactory::MakeTracer(const char* configuration,
                                   std::string& error_message) const
    noexcept try {
  tracer_configuration::TracerConfiguration tracer_configuration;
  auto parse_result = google::protobuf::util::JsonStringToMessage(
      configuration, &tracer_configuration);
  if (!parse_result.ok()) {
    error_message = parse_result.ToString();
    return opentracing::make_unexpected(opentracing::configuration_parse_error);
  }
  auto options = MakeTracerOptions(tracer_configuration);
  auto result = std::shared_ptr<opentracing::Tracer>{
      MakeLightStepTracer(std::move(options))};
  if (result == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_configuration_error);
  }
  return result;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}
}  // namespace lightstep

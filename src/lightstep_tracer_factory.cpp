#include "lightstep_tracer_factory.h"
#include <google/protobuf/util/json_util.h>
#include <lightstep/tracer.h>
#include "lightstep-tracer-configuration/tracer_configuration.pb.h"

namespace lightstep {
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

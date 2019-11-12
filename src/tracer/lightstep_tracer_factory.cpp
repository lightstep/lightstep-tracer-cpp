#include "tracer/lightstep_tracer_factory.h"

#include <iostream>
#include <limits>
#include <sstream>

#include <google/protobuf/util/json_util.h>
#include "lightstep-tracer-configuration/tracer_configuration.pb.h"

#include "tracer/json_options.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
opentracing::expected<std::shared_ptr<opentracing::Tracer>>
LightStepTracerFactory::MakeTracer(const char* configuration,
                                   std::string& error_message) const
    noexcept try {
  auto options_maybe = MakeTracerOptions(configuration, error_message);
  if (!options_maybe) {
    return opentracing::make_unexpected(options_maybe.error());
  }
  auto result = std::shared_ptr<opentracing::Tracer>{
      MakeLightStepTracer(std::move(*options_maybe))};
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

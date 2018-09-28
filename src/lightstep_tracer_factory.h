#pragma once

#include <lightstep/tracer.h>
#include <opentracing/tracer_factory.h>

#include "configuration-proto/tracer_configuration.pb.h"

namespace lightstep {
LightStepTracerOptions MakeTracerOptions(
    const configuration_proto::TracerConfiguration& configuration);

class LightStepTracerFactory final : public opentracing::TracerFactory {
 public:
  opentracing::expected<std::shared_ptr<opentracing::Tracer>> MakeTracer(
      const char* configuration, std::string& error_message) const
      noexcept override;
};
}  // namespace lightstep

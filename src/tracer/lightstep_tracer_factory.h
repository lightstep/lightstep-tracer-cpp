#pragma once

#include <lightstep/tracer.h>

#include <opentracing/tracer_factory.h>

namespace lightstep {
class LightStepTracerFactory final : public opentracing::TracerFactory {
 public:
  // opentracing::TracerFactory
  opentracing::expected<std::shared_ptr<opentracing::Tracer>> MakeTracer(
      const char* configuration, std::string& error_message) const
      noexcept override;
};
}  // namespace lightstep

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

/**
 * Construct LightStepTracerOptions from configuration json.
 * @param configuration the json configuration
 * @param error_message an error message provided upon failure
 * @return the LightStepTracerOptions or an error code
 */
opentracing::expected<LightStepTracerOptions> MakeTracerOptions(
    const char* configuration, std::string& error_message);
}  // namespace lightstep

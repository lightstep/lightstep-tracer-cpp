#pragma once

#include <lightstep/tracer.h>

namespace lightstep {
/**
 * Construct LightStepTracerOptions from configuration json.
 * @param configuration the json configuration
 * @param error_message an error message provided upon failure
 * @return the LightStepTracerOptions or an error code
 */
opentracing::expected<LightStepTracerOptions> MakeTracerOptions(
    const char* configuration, std::string& error_message);
}  // namespace lightstep

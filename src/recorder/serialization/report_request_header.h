#pragma once

#include <cstdint>
#include <string>

#include "lightstep/tracer.h"

namespace lightstep {
/**
 * Serializes the common parts of a ReportRequest.
 * @param tracer_options the options used to construct the tracer.
 * @param reporter_id a unique ID for the reporter.
 * @return a string with the common parts of the ReportRequest serialization.
 */
std::string WriteReportRequestHeader(
    const LightStepTracerOptions& tracer_options, uint64_t reporter_id);
}  // namespace lightstep

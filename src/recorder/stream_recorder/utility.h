#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "common/circular_buffer.h"
#include "lightstep/tracer.h"
#include "network/fragment_array_input_stream.h"

namespace lightstep {
/**
 * Separates a vector of host-port pairs into a bector of unique hosts and
 * indexed endpoints.
 * @param endpoints a vector of host-port pairs.
 * @return a vector of unique hosts and a vector of indexed endpoints.
 */
std::pair<std::vector<const char*>, std::vector<std::pair<int, uint16_t>>>
SeparateEndpoints(
    const std::vector<std::pair<std::string, uint16_t>>& endpoints);

/**
 * Serializes the common parts of a ReportRequest.
 * @param tracer_options the options used to construct the tracer.
 * @param reporter_id a unique ID for the reporter.
 * @return a string with the common parts of the ReportRequest serialization.
 */
std::string WriteStreamHeaderCommonFragment(
    const LightStepTracerOptions& tracer_options, uint64_t reporter_id);

bool Contains(const char* data, size_t size, const char* ptr) noexcept;

void SetSpanFragment(FragmentArrayInputStream& stream,
                     const CircularBufferConstPlacement& placement,
                     const char* position);
}  // namespace lightstep

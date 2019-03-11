#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "lightstep/tracer.h"

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

std::string WriteStreamHeaderCommonSegment(
    const LightStepTracerOptions& tracer_options, uint64_t recorder_id);
}  // namespace lightstep

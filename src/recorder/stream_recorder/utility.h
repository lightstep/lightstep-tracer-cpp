#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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
}  // namespace lightstep

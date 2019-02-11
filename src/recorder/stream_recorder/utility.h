#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace lightstep {
std::pair<std::vector<const char*>, std::vector<std::pair<int, uint16_t>>>
SeparateEndpoints(
    const std::vector<std::pair<std::string, uint16_t>>& endpoints);
}  // namespace lightstep

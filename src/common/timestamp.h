#pragma once

#include <chrono>
#include <cstdint>

namespace lightstep {
int64_t ComputeSystemSteadyTimestampDelta() noexcept;

std::chrono::system_clock::time_point ToSystemTimestamp(
    int64_t timestamp_delta,
    std::chrono::steady_clock::time_point steady_time) noexcept;

std::chrono::steady_clock::time_point ToSteadyTimestamp(
    int64_t timestamp_delta,
    std::chrono::system_clock::time_point system_time) noexcept;
}  // namespace lightstep

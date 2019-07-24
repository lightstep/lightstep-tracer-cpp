#pragma once

#include <chrono>
#include <cstdint>

namespace lightstep {
/**
 * Computes a delta between the steady and system clocks that can be used for
 * timestamp conversions.
 * @return the timestamp delta
 */
int64_t ComputeSystemSteadyTimestampDelta() noexcept;

/**
 * Convert a steady time point to a system time point
 * @param timestamp_delta the system-steady timestamp delta
 * @param steady_timestamp the steady timestamp
 */
std::chrono::system_clock::time_point ToSystemTimestamp(
    int64_t timestamp_delta,
    std::chrono::steady_clock::time_point steady_timestamp) noexcept;

/**
 * Convert a system time point to a steady time point
 * @param timestamp_delta the system-steady timestamp delta
 * @param system_timestamp the system timestamp
 */
std::chrono::steady_clock::time_point ToSteadyTimestamp(
    int64_t timestamp_delta,
    std::chrono::system_clock::time_point system_timestamp) noexcept;
}  // namespace lightstep

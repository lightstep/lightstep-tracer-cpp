#pragma once

#include <chrono>
#include <tuple>

#include "recorder/recorder.h"

#include <opentracing/string_view.h>
#include <opentracing/value.h>

namespace lightstep {
using SystemClock = std::chrono::system_clock;
using SteadyClock = std::chrono::steady_clock;
using SystemTime = SystemClock::time_point;
using SteadyTime = SteadyClock::time_point;

/**
 * @return true if value indicates we should sample.
 */
inline bool is_sampled(const opentracing::Value& value) noexcept {
  return value != opentracing::Value{0} && value != opentracing::Value{0u};
}

/**
 * Compute the start timestamps of a span.
 * @param recorder the recorder
 * @param start_system_timestamp the system start time or SystemTime{} if not
 * set
 * @param start_steady_timestamp the steady start time  or SteadyTime{} if not
 * set
 * @return the start system and steady timestamps
 */
inline std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const Recorder& recorder, const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) noexcept {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    auto steady_now = SteadyClock::now();
    return std::tuple<SystemTime, SteadyTime>{
        recorder.ComputeCurrentSystemTimestamp(steady_now), steady_now};
  }
  if (start_system_timestamp == SystemTime()) {
    auto timestamp_delta = recorder.ComputeSystemSteadyTimestampDelta();
    return std::tuple<SystemTime, SteadyTime>{
        ToSystemTimestamp(timestamp_delta, start_steady_timestamp),
        start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    auto timestamp_delta = recorder.ComputeSystemSteadyTimestampDelta();
    return std::tuple<SystemTime, SteadyTime>{
        start_system_timestamp,
        ToSteadyTimestamp(timestamp_delta, start_system_timestamp)};
  }
  return std::tuple<SystemTime, SteadyTime>{start_system_timestamp,
                                            start_steady_timestamp};
}

/**
 * Appends trace-states to have the union of the two key-value lists.
 * @param trace_state the trace_state to append to
 * @param key_values the key-values of another trace-state
 */
void AppendTraceState(std::string& trace_state,
                      opentracing::string_view key_values);
}  // namespace lightstep

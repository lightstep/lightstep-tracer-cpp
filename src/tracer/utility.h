#pragma once

#include <chrono>
#include <tuple>

#include "recorder/recorder.h"

#include <opentracing/value.h>

namespace lightstep {
using SystemClock = std::chrono::system_clock;
using SteadyClock = std::chrono::steady_clock;
using SystemTime = SystemClock::time_point;
using SteadyTime = SteadyClock::time_point;

bool is_sampled(const opentracing::Value& value) noexcept;

std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const Recorder& recorder, const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) noexcept;
}  // namespace lightstep

#include "tracer/utility.h"

#include <opentracing/util.h>

namespace lightstep {
//------------------------------------------------------------------------------
// is_sampled
//------------------------------------------------------------------------------
#if 0
bool is_sampled(const opentracing::Value& value) noexcept {
  return value != opentracing::Value{0} && value != opentracing::Value{0u};
}
#endif

//------------------------------------------------------------------------------
// ComputeStartTimestamps
//------------------------------------------------------------------------------
#if 0
std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
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
#endif
}  // namespace lightstep

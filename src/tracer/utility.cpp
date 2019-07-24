#include "tracer/utility.h"

#include <opentracing/util.h>

namespace lightstep {
//------------------------------------------------------------------------------
// is_sampled
//------------------------------------------------------------------------------
bool is_sampled(const opentracing::Value& value) noexcept {
  return value != opentracing::Value{0} && value != opentracing::Value{0u};
}

//------------------------------------------------------------------------------
// ComputeStartTimestamps
//------------------------------------------------------------------------------
std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) noexcept {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{SystemClock::now(),
                                              SteadyClock::now()};
    /* return std::tuple<SystemTime, SteadyTime>{SystemTime{}, */
    /*                                           SteadyClock::now()}; */
  }
  if (start_system_timestamp == SystemTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        opentracing::convert_time_point<SystemClock>(start_steady_timestamp),
        start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        start_system_timestamp,
        opentracing::convert_time_point<SteadyClock>(start_system_timestamp)};
  }
  return std::tuple<SystemTime, SteadyTime>{start_system_timestamp,
                                            start_steady_timestamp};
}
}  // namespace lightstep

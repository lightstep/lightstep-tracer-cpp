#include "common/timestamp.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ComputeNanosecondsSinceEpoch
//--------------------------------------------------------------------------------------------------
template <class Clock, class Duration>
static int64_t ComputeNanosecondsSinceEpoch(
    std::chrono::time_point<Clock, Duration> time_point) noexcept {
  return static_cast<int64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          time_point.time_since_epoch())
          .count());
}

//--------------------------------------------------------------------------------------------------
// ComputeSystemSteadyTimestampDelta
//--------------------------------------------------------------------------------------------------
int64_t ComputeSystemSteadyTimestampDelta() noexcept {
  auto steady_now = std::chrono::steady_clock::now();
  auto system_now = std::chrono::system_clock::now();
  return ComputeNanosecondsSinceEpoch(system_now) -
         ComputeNanosecondsSinceEpoch(steady_now);
}

//--------------------------------------------------------------------------------------------------
// ToSystemTimestamp
//--------------------------------------------------------------------------------------------------
std::chrono::system_clock::time_point ToSystemTimestamp(
    int64_t timestamp_delta,
    std::chrono::steady_clock::time_point steady_timestamp) noexcept {
  auto time_since_epoch = std::chrono::nanoseconds{
      ComputeNanosecondsSinceEpoch(steady_timestamp) + timestamp_delta};
  return std::chrono::system_clock::time_point{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(
          time_since_epoch)};
}

//--------------------------------------------------------------------------------------------------
// ToSteadyTimestamp
//--------------------------------------------------------------------------------------------------
std::chrono::steady_clock::time_point ToSteadyTimestamp(
    int64_t timestamp_delta,
    std::chrono::system_clock::time_point system_timestamp) noexcept {
  auto time_since_epoch = std::chrono::nanoseconds{
      ComputeNanosecondsSinceEpoch(system_timestamp) - timestamp_delta};
  return std::chrono::steady_clock::time_point{
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          time_since_epoch)};
}
}  // namespace lightstep

#pragma once

#include <chrono>
#include <memory>

#include "common/serialization_chain.h"
#include "common/timestamp.h"

#include <lightstep/tracer.h>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
// Abstract class that accepts spans from a Tracer once they are finished.
class Recorder {
 public:
  Recorder() noexcept = default;

  Recorder(Recorder&&) = delete;
  Recorder(const Recorder&) = delete;

  virtual ~Recorder() = default;

  Recorder& operator=(Recorder&&) = delete;
  Recorder& operator=(const Recorder&) = delete;

  /**
   * Record a Span
   * @span the protobuf span
   */
  virtual void RecordSpan(const collector::Span& /*span*/) noexcept {}

  /**
   * Record a Span
   * @span the serialization of a protobuf span and framing
   */
  virtual void RecordSpan(
      std::unique_ptr<SerializationChain>&& /*span*/) noexcept {}

  /**
   * Block until the recorder is flushed or a time limit is exceeded.
   * @param timeout the maximum amount of time to block.
   * @return true if the flush was completed.
   */
  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }

  /**
   * Block until the recorder cleanly shuts down its connections to satellites.
   * @param timeout the maximum amount of time to block.
   * @return true if the shutdown was completed.
   */
  virtual bool ShutdownWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }

  /**
   * Compute a timestamp delta that con be used to convert between system and
   * steady timestamps.
   *
   * Having the the recorder provide this functionality
   * allows it to cache and regulary refresh the value to avoid the performance
   * cost of always computing it.
   * @return the timestamp delta
   */
  virtual int64_t ComputeSystemSteadyTimestampDelta() const noexcept {
    return lightstep::ComputeSystemSteadyTimestampDelta();
  }

  /**
   * Compute the current system time from current steady time point.
   *
   * If the recorder caches a timestamp delta, it can avoid a call to
   * system_clock::now.
   * @param steady_now the current steady timestamp
   * @return the current system timestamp
   */
  virtual std::chrono::system_clock::time_point ComputeCurrentSystemTimestamp(
      std::chrono::steady_clock::time_point /*steady_now*/) const noexcept {
    return std::chrono::system_clock::now();
  }

  /**
   * Accessor to metrics observer if available.
   * @return a pointer to the attached MetricsObserver or nullptr
   */
  virtual const MetricsObserver* metrics_observer() const noexcept {
    return nullptr;
  }
};
}  // namespace lightstep

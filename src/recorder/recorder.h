#pragma once

#include <chrono>
#include <memory>

#include "common/serialization_chain.h"

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

  /* virtual std::chrono::duration::nanoseconds system_steady_timestamp_delta() noexcept { */
  /* } */

  virtual std::chrono::system_clock::time_point ComputeCurrentSystemTime(
      std::chrono::steady_clock::time_point /*steady_now*/) noexcept {
    return std::chrono::system_clock::now();
  }
};
}  // namespace lightstep

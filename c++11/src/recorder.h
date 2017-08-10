#pragma once

#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <chrono>

namespace lightstep {
/**
 * Abstract class that accepts spans from a Tracer once they are finished.
 */
class Recorder {
 public:
  virtual ~Recorder() = default;

  virtual void RecordSpan(collector::Span&& span) noexcept = 0;

  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }
};
}  // namespace lightstep

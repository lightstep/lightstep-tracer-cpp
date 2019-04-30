#pragma once

#include <lightstep/tracer.h>
#include <chrono>
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

  virtual void RecordSpan(const collector::Span& span) noexcept = 0;

  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }
};
}  // namespace lightstep

#pragma once

#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <chrono>
#include "transporter.h"

namespace lightstep {
class Recorder {
 public:
  virtual ~Recorder() = default;

  virtual void RecordSpan(collector::Span&& span) noexcept = 0;

  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration /*timeout*/) noexcept {
    return true;
  }
};

std::unique_ptr<Recorder> make_rpc_recorder(
    const LightStepTracerOptions& options,
    std::unique_ptr<Transporter>&& transporter);

std::unique_ptr<Recorder> make_lightstep_recorder(
    const LightStepTracerOptions& options) noexcept;
}  // namespace lightstep

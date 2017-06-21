#ifndef RECORDER_H
#define RECORDER_H

#include <collector.pb.h>
#include <chrono>

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

std::unique_ptr<Recorder> make_lightstep_recorder(
    const LightStepTracerOptions& options) noexcept;
}  // namespace lightstep

#endif  // RECORDER_H

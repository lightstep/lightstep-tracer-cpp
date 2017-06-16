#ifndef RECORDER_H
#define RECORDER_H

#include <collector.pb.h>
#include <chrono>

namespace lightstep {
//------------------------------------------------------------------------------
// Recorder
//------------------------------------------------------------------------------
class Recorder {
 public:
  virtual ~Recorder() = default;

  virtual void RecordSpan(collector::Span&& span) noexcept = 0;

  virtual bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept {
    return true;
  }
};
}  // namespace lightstep

#endif  // RECORDER_H

#pragma once

#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/transporter.h"
#include "recorder/fork_aware_recorder.h"

namespace lightstep {
class ManualRecorder : public ForkAwareRecorder, private Noncopyable {
 public:
  ManualRecorder(Logger& logger, LightStepTracerOptions options,
                 std::unique_ptr<AsyncTransporter>&& transporter);

  // Recorder
  void RecordSpan(std::unique_ptr<SerializationChain>&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  // ForkAwareRecorder
  void PrepareForFork() noexcept override;

  void OnForkedParent() noexcept override;

  void OnForkedChild() noexcept override;

 private:
};
}  // namespace lightstep

#pragma once

#include "common/circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/transporter.h"
#include "recorder/fork_aware_recorder.h"
#include "recorder/metrics_tracker.h"

namespace lightstep {
class ManualRecorder : public ForkAwareRecorder, private Noncopyable {
 public:
  ManualRecorder(Logger& logger, LightStepTracerOptions options,
                 std::unique_ptr<AsyncTransporter>&& transporter);

  // Recorder
  Fragment ReserveHeaderSpace(ChainedStream& stream) override;

  void RecordSpan(Fragment header_fragment,
                  std::unique_ptr<ChainedStream>&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  // ForkAwareRecorder
  void PrepareForFork() noexcept override;

  void OnForkedParent() noexcept override;

  void OnForkedChild() noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  std::unique_ptr<AsyncTransporter> transporter_;

  MetricsTracker metrics_;
  CircularBuffer<ChainedStream> span_buffer_;
};
}  // namespace lightstep

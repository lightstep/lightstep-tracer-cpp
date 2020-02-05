#pragma once

#include <mutex>

#include "common/circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/transporter.h"
#include "recorder/fork_aware_recorder.h"
#include "recorder/metrics_tracker.h"

namespace lightstep {
/**
 * Implements a Recorder with no background thread and custom Transporter.
 */
class ManualRecorder : public ForkAwareRecorder,
                       public AsyncTransporter::Callback,
                       private Noncopyable {
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
  void OnForkedChild() noexcept override;

  // AsyncTransporter::Callback
  void OnSuccess(BufferChain& message) noexcept override;

  void OnFailure(BufferChain& message) noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  std::unique_ptr<AsyncTransporter> transporter_;
  std::shared_ptr<const std::string> report_request_header_;

  MetricsTracker metrics_;
  std::mutex flush_mutex_;
  CircularBuffer<ChainedStream> span_buffer_;
};
}  // namespace lightstep

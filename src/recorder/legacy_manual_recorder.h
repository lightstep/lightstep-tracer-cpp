#pragma once

#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/transporter.h"
#include "recorder/recorder.h"
#include "recorder/report_builder.h"

namespace lightstep {
// LegacyManualRecorder buffers spans finished by a tracer and sends them over
// to the provided LegacyAsyncTransporter when FlushWithTimeout is called.
class LegacyManualRecorder final : public Recorder,
                                   private LegacyAsyncTransporter::Callback,
                                   private Noncopyable {
 public:
  LegacyManualRecorder(Logger& logger, LightStepTracerOptions options,
                       std::unique_ptr<LegacyAsyncTransporter>&& transporter);

  void RecordSpan(const collector::Span& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  bool IsReportInProgress() const noexcept;

  bool FlushOne() noexcept;

  void OnSuccess() noexcept override;
  void OnFailure(std::error_code error) noexcept override;

  Logger& logger_;
  LightStepTracerOptions options_;

  bool disabled_ = false;

  // Buffer state
  ReportBuilder builder_;
  collector::ReportRequest active_request_;
  collector::ReportResponse active_response_;
  size_t saved_dropped_spans_ = 0;
  size_t saved_pending_spans_ = 0;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;

  // LegacyAsyncTransporter through which to send span reports.
  std::unique_ptr<LegacyAsyncTransporter> transporter_;
};
}  // namespace lightstep

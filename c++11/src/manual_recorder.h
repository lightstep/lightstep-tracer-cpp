#pragma once

#include <lightstep/transporter.h>
#include "recorder.h"
#include "report_builder.h"

namespace lightstep {
class ManualRecorder : public Recorder {
 public:
  explicit ManualRecorder(LightStepManualTracerOptions options);

  void RecordSpan(collector::Span&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  void FlushOne();

  static void OnSuccessCallback(void* context);
  static void OnFailureCallback(std::error_code error, void* context);

  LightStepManualTracerOptions options_;

  // Buffer state (protected by write_mutex_).
  ReportBuilder builder_;
  collector::ReportRequest active_request_;
  collector::ReportResponse active_response_;
  size_t saved_dropped_spans_ = 0;
  size_t saved_pending_spans_ = 0;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;
};
}  // namespace lightstep

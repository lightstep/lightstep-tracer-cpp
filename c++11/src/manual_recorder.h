#pragma once

#include <lightstep/transporter.h>
#include "recorder.h"
#include "report_builder.h"

namespace lightstep {
class ManualRecorder : public Recorder {
 public:
  explicit ManualRecorder(LightStepManualTracerOptions options);

  /* ManualRecorder(LightStepManualTracerOptions options); */
  void RecordSpan(collector::Span&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  LightStepManualTracerOptions options_;
};
}  // namespace lightstep

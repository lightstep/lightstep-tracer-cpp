#pragma once

#include "lightstep/tracer.h"
#include "common/logger.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
class StreamRecorder final : public Recorder {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  // Recorder
  void RecordSpan(const collector::Span& span) noexcept override;
 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
};
} // namespace lightstep

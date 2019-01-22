#include "stream_recorder.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
StreamRecorder::StreamRecorder(Logger& logger,
                               LightStepTracerOptions&& tracer_options,
                               StreamRecorderOptions&& recorder_options)
    : logger_{logger},
      tracer_options_{std::move(tracer_options)},
      recorder_options_{std::move(recorder_options)} {
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void StreamRecorder::RecordSpan(const collector::Span& /*span*/) noexcept {
}

//--------------------------------------------------------------------------------------------------
// MakeStreamRecorder
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Recorder> MakeStreamRecorder(
    Logger& logger, LightStepTracerOptions&& tracer_options) {
  std::unique_ptr<Recorder> result{
      new StreamRecorder{logger, std::move(tracer_options)}};
  return result;
}
} // namespace lightstep

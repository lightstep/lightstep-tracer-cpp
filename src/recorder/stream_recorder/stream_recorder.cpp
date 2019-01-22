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
      recorder_options_{std::move(recorder_options)},
      span_buffer_{recorder_options_.max_span_buffer_bytes},
      poll_timer_{event_base_, recorder_options_.polling_period,
                  MakeTimerCallback<&StreamRecorder::Poll>(),
                  static_cast<void*>(this)} {
  thread_ = std::thread{&StreamRecorder::Run, this};
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
StreamRecorder::~StreamRecorder() noexcept {
  exit_ = true;
  thread_.join();
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void StreamRecorder::RecordSpan(const collector::Span& /*span*/) noexcept {
}

//--------------------------------------------------------------------------------------------------
// Run
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Run() noexcept {
  event_base_.Dispatch();
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Poll() noexcept {
  if (exit_) {
    event_base_.LoopBreak();
  }
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

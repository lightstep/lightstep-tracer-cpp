#include "recorder/stream_recorder/stream_recorder2.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
StreamRecorder2::StreamRecorder2(Logger& logger,
                                 LightStepTracerOptions&& tracer_options,
                                 StreamRecorderOptions&& recorder_options) {
  (void)logger;
  (void)tracer_options;
  (void)recorder_options;
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::RecordSpan(std::unique_ptr<SerializationChain>&& span) noexcept {
  (void)span;
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool StreamRecorder2::FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept {
  (void)timeout;
  return true;
}
} // namespace lightstep

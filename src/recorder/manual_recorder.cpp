#include "recorder/manual_recorder.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(
    Logger& logger, LightStepTracerOptions options,
    std::unique_ptr<AsyncTransporter>&& transporter) {
  (void)logger;
  (void)options;
  (void)transporter;
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(
    std::unique_ptr<SerializationChain>&& span) noexcept {
  (void)span;
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept {
  return true;
}

//--------------------------------------------------------------------------------------------------
// PrepareForFork
//--------------------------------------------------------------------------------------------------
void ManualRecorder::PrepareForFork() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnForkedParent
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnForkedParent() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnForkedChild
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnForkedChild() noexcept {}
}  // namespace lightstep

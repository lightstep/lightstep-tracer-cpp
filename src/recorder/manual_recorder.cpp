#include "recorder/manual_recorder.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(Logger& logger, LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      tracer_options_{std::move(options)},
      transporter_{std::move(transporter)},
      span_buffer_{tracer_options_.max_buffered_spans.value()} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (tracer_options_.metrics_observer == nullptr) {
    tracer_options_.metrics_observer.reset(new MetricsObserver{});
  }
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(
    std::unique_ptr<SerializationChain>&& span) noexcept {
  span->AddFraming();
  if (!span_buffer_.Add(span)) {
    tracer_options_.metrics_observer->OnSpansDropped(1);
    span.reset();
  }
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

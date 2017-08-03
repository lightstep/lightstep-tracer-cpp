#include "manual_recorder.h"
#include "logger.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(LightStepManualTracerOptions options)
    : options_{std::move(options)},
      builder_{options_.access_token, options_.tags} {}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(collector::Span&& span) noexcept {
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    dropped_spans_++;
    return;
  }
  builder_.AddSpan(std::move(span));
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
void ManualRecorder::FlushOne() {
  // If a report is currently in flight, do nothing.
  if (encoding_seqno_ > 1 + flushed_seqno_) {
    return;
  }

  saved_pending_spans_ = builder_.num_pending_spans();
  if (saved_pending_spans_ == 0) {
    return;
  }
  saved_dropped_spans_ = dropped_spans_;
  builder_.set_pending_client_dropped_spans(dropped_spans_);
  dropped_spans_ = 0;
  std::swap(builder_.pending(), active_request_);
  ++encoding_seqno_;
  options_.transporter->Send(active_request_, active_response_,
                             OnSuccessCallback, OnFailureCallback,
                             static_cast<void*>(this));
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept {
  FlushOne();
  return true;
}

//------------------------------------------------------------------------------
// OnSuccessCallback
//------------------------------------------------------------------------------
void ManualRecorder::OnSuccessCallback(void* context) {
  auto recorder = static_cast<ManualRecorder*>(context);
  ++recorder->flushed_seqno_;
  recorder->active_request_.Clear();
  if (recorder->options_.verbose) {
    GetLogger().info(R"(Report: resp="{}")",
                     recorder->active_response_.ShortDebugString());
  }
}

//------------------------------------------------------------------------------
// OnFailureCallback
//------------------------------------------------------------------------------
void ManualRecorder::OnFailureCallback(std::error_code error, void* context) {
  auto recorder = static_cast<ManualRecorder*>(context);
  ++recorder->flushed_seqno_;
  recorder->active_request_.Clear();
  recorder->dropped_spans_ +=
      recorder->saved_dropped_spans_ + recorder->saved_pending_spans_;
  GetLogger().error("Failed to send report: {}", error.message());
}
}  // namespace lightstep

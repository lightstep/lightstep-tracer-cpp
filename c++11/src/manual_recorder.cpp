#include "manual_recorder.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(spdlog::logger& logger,
                               LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      options_{std::move(options)},
      builder_{options_.access_token, options_.tags},
      transporter_{std::move(transporter)} {}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(collector::Span&& span) noexcept {
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    dropped_spans_++;
    return;
  }
  builder_.AddSpan(std::move(span));
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    FlushOne();
  }
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
bool ManualRecorder::FlushOne() {
  // If a report is currently in flight, do nothing; and if there are any
  // pending spans, then the flush is considered to have failed.
  if (encoding_seqno_ > 1 + flushed_seqno_) {
    return builder_.num_pending_spans() == 0;
  }

  saved_pending_spans_ = builder_.num_pending_spans();
  if (saved_pending_spans_ == 0) {
    return true;
  }
  saved_dropped_spans_ = dropped_spans_;
  builder_.set_pending_client_dropped_spans(dropped_spans_);
  dropped_spans_ = 0;
  std::swap(builder_.pending(), active_request_);
  ++encoding_seqno_;
  transporter_->Send(active_request_, active_response_, *this);
  return true;
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept {
  return FlushOne();
}

//------------------------------------------------------------------------------
// OnSuccess
//------------------------------------------------------------------------------
void ManualRecorder::OnSuccess() noexcept {
  ++flushed_seqno_;
  active_request_.Clear();
  if (options_.verbose) {
    logger_.info(R"(Report: resp="{}")", active_response_.ShortDebugString());
  }
}

//------------------------------------------------------------------------------
// OnFailure
//------------------------------------------------------------------------------
void ManualRecorder::OnFailure(std::error_code error) noexcept {
  ++flushed_seqno_;
  active_request_.Clear();
  dropped_spans_ += saved_dropped_spans_ + saved_pending_spans_;
  logger_.error("Failed to send report: {}", error.message());
}
}  // namespace lightstep

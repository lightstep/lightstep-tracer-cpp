#include "manual_recorder.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(Logger& logger, LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      options_{std::move(options)},
      builder_{options_.access_token, options_.tags},
      transporter_{std::move(transporter)} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (options_.metrics_observer == nullptr) {
    options_.metrics_observer.reset(new MetricsObserver{});
  }
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(collector::Span&& span) noexcept try {
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    dropped_spans_++;
    options_.metrics_observer->OnSpansDropped(1);
    return;
  }
  builder_.AddSpan(std::move(span));
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    FlushOne();
  }
} catch (const std::exception& e) {
  logger_.Error("Failed to record span: ", e.what());
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
bool ManualRecorder::FlushOne() noexcept try {
  options_.metrics_observer->OnFlush();

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
} catch (const std::exception& e) {
  logger_.Error("Failed to Flush: ", e.what());
  options_.metrics_observer->OnSpansDropped(saved_pending_spans_);
  dropped_spans_ += saved_pending_spans_;
  active_request_.Clear();
  return false;
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
  options_.metrics_observer->OnSpansSent(saved_pending_spans_);
  if (options_.verbose) {
    logger_.Info(R"(Report: resp=")", active_response_.ShortDebugString(),
                 R"(")");
  }
}

//------------------------------------------------------------------------------
// OnFailure
//------------------------------------------------------------------------------
void ManualRecorder::OnFailure(std::error_code error) noexcept {
  ++flushed_seqno_;
  active_request_.Clear();
  options_.metrics_observer->OnSpansDropped(saved_pending_spans_);
  dropped_spans_ += saved_dropped_spans_ + saved_pending_spans_;
  logger_.Error("Failed to send report: ", error.message());
}
}  // namespace lightstep

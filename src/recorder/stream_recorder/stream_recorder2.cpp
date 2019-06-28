#include "recorder/stream_recorder/stream_recorder2.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GetMetricsObserver
//--------------------------------------------------------------------------------------------------
static MetricsObserver& GetMetricsObserver(
    LightStepTracerOptions& tracer_options) {
  if (tracer_options.metrics_observer == nullptr) {
    tracer_options.metrics_observer.reset(new MetricsObserver{});
  }
  return *tracer_options.metrics_observer;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
StreamRecorder2::StreamRecorder2(Logger& logger,
                                 LightStepTracerOptions&& tracer_options,
                                 StreamRecorderOptions&& recorder_options)
    : logger_{logger},
      tracer_options_{std::move(tracer_options)},
      recorder_options_{std::move(recorder_options)},
      metrics_{GetMetricsObserver(tracer_options_)},
      span_buffer_{tracer_options_.max_buffered_spans.value()} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (tracer_options_.metrics_observer == nullptr) {
    tracer_options_.metrics_observer.reset(new MetricsObserver{});
  }
  stream_recorder_impl_.reset(new StreamRecorderImpl2{*this});
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::RecordSpan(
    std::unique_ptr<SerializationChain>&& span) noexcept {
  span->AddFraming();
  if (!span_buffer_.Add(span)) {
    logger_.Debug("Dropping span");
    metrics_.OnSpansDropped(1);
    span.reset();
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool StreamRecorder2::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept try {
  auto num_spans_produced = span_buffer_.production_count();
  std::unique_lock<std::mutex> lock{flush_mutex_};
  if (num_spans_consumed_ >= num_spans_produced) {
    return true;
  }
  ++pending_flush_counter_;
  flush_condition_variable_.wait_for(lock, timeout, [this, num_spans_produced] {
    return exit_ || num_spans_consumed_ >= num_spans_produced;
  });
  return num_spans_consumed_ >= num_spans_produced;
} catch (const std::exception& e) {
  logger_.Error("StreamRecorder::FlushWithTimeout failed: ", e.what());
  return false;
}

//--------------------------------------------------------------------------------------------------
// PrepareForFork
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::PrepareForFork() noexcept {
  // We don't want parent and child processes to share sockets so close any open
  // connections.
  stream_recorder_impl_.reset(nullptr);
}

//--------------------------------------------------------------------------------------------------
// OnForkedParent
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::OnForkedParent() noexcept {
  stream_recorder_impl_.reset(new StreamRecorderImpl2{*this});
}

//--------------------------------------------------------------------------------------------------
// OnForkedChild
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::OnForkedChild() noexcept {
  // Clear any buffered data since it will already be recorded from the parent
  // process.
  metrics_.ConsumeDroppedSpans();
  span_buffer_.Clear();
  num_spans_consumed_ = span_buffer_.production_count();
  pending_flush_counter_ = 0;

  stream_recorder_impl_.reset(new StreamRecorderImpl2{*this});
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorder2::Poll() noexcept {
  auto num_spans_consumed = span_buffer_.consumption_count();
  if (num_spans_consumed > num_spans_consumed_) {
    {
      std::lock_guard<std::mutex> lock_guard{flush_mutex_};
      num_spans_consumed_ = num_spans_consumed;
    }
    flush_condition_variable_.notify_all();
  }
}
}  // namespace lightstep

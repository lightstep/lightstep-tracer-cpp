#include "recorder/stream_recorder/stream_recorder.h"

#include <exception>

#include "common/protobuf.h"

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
StreamRecorder::StreamRecorder(Logger& logger,
                               LightStepTracerOptions&& tracer_options,
                               StreamRecorderOptions&& recorder_options)
    : logger_{logger},
      tracer_options_{std::move(tracer_options)},
      recorder_options_{std::move(recorder_options)},
      metrics_{GetMetricsObserver(tracer_options_)},
      span_buffer_{tracer_options_.max_buffered_spans.value()} {
  stream_recorder_impl_.reset(new StreamRecorderImpl{*this});
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
StreamRecorder::~StreamRecorder() noexcept {
  {
    std::lock_guard<std::mutex> lock_guard{flush_mutex_};
    exit_ = true;
  }
  flush_condition_variable_.notify_all();
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void StreamRecorder::RecordSpan(
    std::unique_ptr<SerializationChain>&& span) noexcept {
  span->AddFraming();
  if (!span_buffer_.Add(span)) {
    // Note: the compiler doesn't want to inline this logger call and it shows
    // up in profiling with high span droppage even if the logging isn't turned
    // on.
    //
    // Hence, the additional checking to avoid a function call.
    if (static_cast<int>(logger_.level()) <=
        static_cast<int>(LogLevel::debug)) {
      logger_.Debug("Dropping span");
    }
    metrics_.OnSpansDropped(1);
    span.reset();
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool StreamRecorder::FlushWithTimeout(
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
// ShutdownWithTimeout
//--------------------------------------------------------------------------------------------------
bool StreamRecorder::ShutdownWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept try {
  std::unique_lock<std::mutex> lock{flush_mutex_};
  ++shutdown_counter_;
  shutdown_condition_variable_.wait_for(
      lock, timeout, [this] { return exit_ || !last_is_active_; });
  return !last_is_active_;
} catch (const std::exception& e) {
  logger_.Error("StreamRecorder::FlushWithTimeout failed: ", e.what());
  return false;
}

//--------------------------------------------------------------------------------------------------
// PrepareForFork
//--------------------------------------------------------------------------------------------------
void StreamRecorder::PrepareForFork() noexcept {
  // We don't want parent and child processes to share sockets so close any open
  // connections.
  stream_recorder_impl_.reset(nullptr);
}

//--------------------------------------------------------------------------------------------------
// OnForkedParent
//--------------------------------------------------------------------------------------------------
void StreamRecorder::OnForkedParent() noexcept {
  stream_recorder_impl_.reset(new StreamRecorderImpl{*this});
}

//--------------------------------------------------------------------------------------------------
// OnForkedChild
//--------------------------------------------------------------------------------------------------
void StreamRecorder::OnForkedChild() noexcept {
  // Clear any buffered data since it will already be recorded from the parent
  // process.
  metrics_.ConsumeDroppedSpans();
  span_buffer_.Clear();
  num_spans_consumed_ = span_buffer_.production_count();
  pending_flush_counter_ = 0;

  stream_recorder_impl_.reset(new StreamRecorderImpl{*this});
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Poll() noexcept {
  auto num_spans_consumed = span_buffer_.consumption_count();
  if (num_spans_consumed > num_spans_consumed_) {
    {
      std::lock_guard<std::mutex> lock_guard{flush_mutex_};
      num_spans_consumed_ = num_spans_consumed;
    }
    flush_condition_variable_.notify_all();
  }

  if (shutdown_counter_.exchange(0) > 0) {
    stream_recorder_impl_->InitiateShutdown();
  }
  
  if (last_is_active_ && !stream_recorder_impl_->is_active()) {
    {
      std::lock_guard<std::mutex> lock_guard{shutdown_mutex_};
      last_is_active_ = false;
    }
    shutdown_condition_variable_.notify_all();
  }
}

//--------------------------------------------------------------------------------------------------
// MakeStreamRecorder
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Recorder> MakeStreamRecorder(
    Logger& logger, LightStepTracerOptions&& tracer_options) {
  return std::unique_ptr<Recorder>{
      new StreamRecorder{logger, std::move(tracer_options)}};
}
}  // namespace lightstep

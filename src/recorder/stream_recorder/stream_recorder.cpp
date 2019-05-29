#include "stream_recorder.h"

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
      span_buffer_{recorder_options_.max_span_buffer_bytes} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (tracer_options_.metrics_observer == nullptr) {
    tracer_options_.metrics_observer.reset(new MetricsObserver{});
  }
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
void StreamRecorder::RecordSpan(const collector::Span& span) noexcept {
  auto span_size = span.ByteSizeLong();
  auto serialization_callback =
      [&span, span_size](google::protobuf::io::CodedOutputStream& stream) {
        WriteEmbeddedMessage(stream,
                             collector::ReportRequest::kSpansFieldNumber,
                             span_size, span);
      };
  auto was_added = span_buffer_.Add(
      serialization_callback,
      ComputeEmbeddedMessageSerializationSize(
          collector::ReportRequest::kSpansFieldNumber, span_size));
  if (!was_added) {
    logger_.Debug("Dropping span ", span.span_context().span_id());
    metrics_.OnSpansDropped(1);
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool StreamRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept try {
  auto num_bytes_produced = span_buffer_.buffer().num_bytes_produced();
  std::unique_lock<std::mutex> lock{flush_mutex_};
  if (num_bytes_consumed_ >= num_bytes_produced) {
    return true;
  }
  ++pending_flush_counter_;
  flush_condition_variable_.wait_for(lock, timeout, [this, num_bytes_produced] {
    return exit_ || num_bytes_consumed_ >= num_bytes_produced;
  });
  return num_bytes_consumed_ >= num_bytes_produced;
} catch (const std::exception& e) {
  logger_.Error("StreamRecorder::FlushWithTimeout failed: ", e.what());
  return false;
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Poll() noexcept {
  auto num_bytes_consumed = span_buffer_.buffer().num_bytes_consumed();
  if (num_bytes_consumed > num_bytes_consumed_) {
    {
      std::lock_guard<std::mutex> lock_guard{flush_mutex_};
      num_bytes_consumed_ = num_bytes_consumed;
    }
    flush_condition_variable_.notify_all();
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
}  // namespace lightstep

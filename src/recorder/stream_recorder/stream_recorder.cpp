#include "stream_recorder.h"

#include <exception>

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
      early_flush_marker_{
          static_cast<size_t>(recorder_options_.max_span_buffer_bytes *
                              recorder_options.early_flush_threshold)},
      poll_timer_{event_base_, recorder_options_.polling_period,
                  MakeTimerCallback<StreamRecorder, &StreamRecorder::Poll>(),
                  static_cast<void*>(this)},
      flush_timer_{event_base_, recorder_options.flushing_period,
                   MakeTimerCallback<StreamRecorder, &StreamRecorder::Flush>(),
                   static_cast<void*>(this)},
      streamer_{logger_, event_base_, tracer_options_, recorder_options_,
                span_buffer_} {
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
void StreamRecorder::RecordSpan(const collector::Span& span) noexcept {
  auto serialization_callback =
      [](google::protobuf::io::CodedOutputStream& stream, size_t /*size*/,
         void* context) {
        static_cast<collector::Span*>(context)->SerializeWithCachedSizes(
            &stream);
      };
  auto was_added =
      span_buffer_.Add(serialization_callback, span.ByteSizeLong(),
                       static_cast<void*>(const_cast<collector::Span*>(&span)));
  if (!was_added) {
    logger_.Debug("Dropping span ", span.span_context().span_id());
  }
}

//--------------------------------------------------------------------------------------------------
// Run
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Run() noexcept try {
  event_base_.Dispatch();
} catch (const std::exception& e) {
  logger_.Error("StreamRecorder::Run failed: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Poll() noexcept {
  if (exit_) {
    try {
      return event_base_.LoopBreak();
    } catch (const std::exception& e) {
      logger_.Error("StreamRecorder: failed to break out of event loop: ",
                    e.what());
      std::terminate();
    }
  }
  if (span_buffer_.size() > early_flush_marker_) {
    Flush();
  }
}

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
void StreamRecorder::Flush() noexcept try {
  streamer_.Flush();
  flush_timer_.Reset();
} catch (const std::exception& e) {
  logger_.Error("StreamRecorder::Flush failed: ", e.what());
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

#include "stream_recorder_impl.h"

#include "recorder/stream_recorder/stream_recorder.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
StreamRecorderImpl::StreamRecorderImpl(StreamRecorder& stream_recorder)
    : stream_recorder_{stream_recorder},
      early_flush_marker_{static_cast<size_t>(
          stream_recorder_.tracer_options().max_buffered_spans.value() *
          stream_recorder_.recorder_options().early_flush_threshold)},
      poll_timer_{
          event_base_, stream_recorder_.recorder_options().polling_period,
          MakeTimerCallback<StreamRecorderImpl, &StreamRecorderImpl::Poll>(),
          static_cast<void*>(this)},
      timestamp_delta_timer_{
          event_base_,
          stream_recorder_.recorder_options().timestamp_delta_period,
          MakeTimerCallback<StreamRecorderImpl,
                            &StreamRecorderImpl::RefreshTimestampDelta>(),
          static_cast<void*>(this)},
      flush_timer_{
          event_base_, stream_recorder_.tracer_options().reporting_period,
          MakeTimerCallback<StreamRecorderImpl, &StreamRecorderImpl::Flush>(),
          static_cast<void*>(this)},
      streamer_{stream_recorder_.logger(),
                event_base_,
                stream_recorder_.tracer_options(),
                stream_recorder_.recorder_options(),
                stream_recorder_.metrics(),
                stream_recorder_.span_buffer()} {
  thread_ = std::thread{&StreamRecorderImpl::Run, this};
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
StreamRecorderImpl::~StreamRecorderImpl() noexcept {
  exit_ = true;
  thread_.join();
}

//--------------------------------------------------------------------------------------------------
// Run
//--------------------------------------------------------------------------------------------------
void StreamRecorderImpl::Run() noexcept try {
  event_base_.Dispatch();
} catch (const std::exception& e) {
  stream_recorder_.logger().Error("StreamRecorder::Run failed: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// Poll
//--------------------------------------------------------------------------------------------------
void StreamRecorderImpl::Poll() noexcept {
  if (exit_) {
    try {
      // Attempt to flush any pending spans before shutting down.
      //
      // Note: Flush is non-blocking so we can do this without causing an
      // unacceptable delay to process termination.
      Flush();

      // Shut down the event loop.
      return event_base_.LoopBreak();
    } catch (const std::exception& e) {
      stream_recorder_.logger().Error(
          "StreamRecorder: failed to break out of event loop: ", e.what());
      std::terminate();
    }
  }

  auto pending_flush_count = stream_recorder_.ConsumePendingFlushCount();
  if (pending_flush_count > 0 ||
      stream_recorder_.span_buffer().size() > early_flush_marker_) {
    Flush();
  }

  stream_recorder_.Poll(*this);
}

//--------------------------------------------------------------------------------------------------
// RefreshTimestampDelta
//--------------------------------------------------------------------------------------------------
void StreamRecorderImpl::RefreshTimestampDelta() noexcept {
  timestamp_delta_ = ComputeSystemSteadyTimestampDelta();
}

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
void StreamRecorderImpl::Flush() noexcept try {
  auto& span_buffer = stream_recorder_.span_buffer();
  if (stream_recorder_.recorder_options().throw_away_spans) {
    span_buffer.Clear();
  } else {
    streamer_.Flush();
  }
  flush_timer_.Reset();
  stream_recorder_.metrics().OnFlush();
} catch (const std::exception& e) {
  stream_recorder_.logger().Error("StreamRecorder::Flush failed: ", e.what());
}
}  // namespace lightstep

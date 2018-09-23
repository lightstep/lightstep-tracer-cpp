#include "stream_recorder.h"

#include "lightstep-tracer-common/collector.pb.h"
#include "utility.h"

#include <chrono>
#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// MakeStreamInitializationMessage
//------------------------------------------------------------------------------
static collector::StreamInitialization MakeStreamInitializationMessage(
    const LightStepTracerOptions& options) {
  collector::StreamInitialization initialization;
  initialization.set_reporter_id(GenerateId());
  initialization.mutable_auth()->set_access_token(options.access_token);
  auto& tags = *initialization.mutable_tags();
  tags.Reserve(static_cast<int>(options.tags.size()));
  for (const auto& tag : options.tags) {
    *tags.Add() = ToKeyValue(tag.first, tag.second);
  }
  return initialization;
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamRecorder::StreamRecorder(Logger& logger, LightStepTracerOptions&& options,
                               std::unique_ptr<StreamTransporter>&& transporter)
    : logger_{logger},
      options_{std::move(options)},
      transporter_{std::move(transporter)},
      message_buffer_{options_.message_buffer_size} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (options_.metrics_observer == nullptr) {
    options_.metrics_observer.reset(new MetricsObserver{});
  }

  notification_threshold_ =
      static_cast<size_t>(options.message_buffer_size * 0.10);
  streamer_thread_ = std::thread{&StreamRecorder::RunStreamer, this};
  if (!message_buffer_.Add(PacketType::Initiation,
                           MakeStreamInitializationMessage(options_))) {
    throw std::runtime_error{"buffer size is too small"};
  }
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamRecorder::~StreamRecorder() {
  MakeStreamerExit();
  streamer_thread_.join();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void StreamRecorder::RecordSpan(const collector::Span& span) noexcept {
  if (!message_buffer_.Add(PacketType::Span, span)) {
    ++num_dropped_spans_;
    options_.metrics_observer->OnSpansDropped(1);
  }

  // notifying the condition variable has a signficant performance cost, so
  // make an effort to make sure it's worthwhile before doing so.
  if (waiting_ &&
      message_buffer_.num_pending_bytes() >= notification_threshold_) {
    condition_variable_.notify_all();
  }
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool StreamRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept try {
  std::unique_lock<std::mutex> lock{mutex_};
  auto consumption_target = message_buffer_.total_bytes_consumed() +
                            message_buffer_.num_pending_bytes();
  auto rcode =
      condition_variable_.wait_for(lock, timeout, [consumption_target, this] {
        return this->exit_streamer_ ||
               this->message_buffer_.total_bytes_consumed() >=
                   consumption_target;
      });
  if (!rcode) {
    return false;
  }
  return message_buffer_.total_bytes_consumed() >= consumption_target;
} catch (const std::exception& e) {
  logger_.Error("Failed to flush recorder: ", e.what());
  return false;
}

//------------------------------------------------------------------------------
// RunStreamer
//------------------------------------------------------------------------------
void StreamRecorder::RunStreamer() noexcept try {
  while (SleepForNextPoll()) {
    while (true) {
      // Keep uploading messages to the satellite until the buffer is empty.
      while (true) {
        if (!message_buffer_.Consume(StreamRecorder::Consume,
                                     static_cast<void*>(this))) {
          break;
        }
      }

      // If there are any relevant stats to send, then upload a metrics report.
      if (!UpdateMetricsReport()) {
        break;
      }
    }
    condition_variable_.notify_all();
  }
} catch (const std::exception& e) {
  logger_.Error("Unexpected streamer error: ", e.what());
  MakeStreamerExit();
}

//------------------------------------------------------------------------------
// MakeStreamerExit
//------------------------------------------------------------------------------
void StreamRecorder::MakeStreamerExit() noexcept {
  {
    std::unique_lock<std::mutex> lock{mutex_};
    exit_streamer_ = true;
  }
  condition_variable_.notify_all();
}

//------------------------------------------------------------------------------
// StreamRecorder
//------------------------------------------------------------------------------
bool StreamRecorder::SleepForNextPoll() {
  std::unique_lock<std::mutex> lock{mutex_};
  waiting_ = true;
  condition_variable_.wait_for(lock, options_.reporting_period, [this] {
    return this->exit_streamer_ || !this->message_buffer_.empty();
  });
  waiting_ = false;
  return !exit_streamer_;
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
size_t StreamRecorder::Consume(void* context, const char* data,
                               size_t num_bytes) {
  auto& stream_recorder = *static_cast<StreamRecorder*>(context);
  return stream_recorder.transporter_->Write(data, num_bytes);
}

//------------------------------------------------------------------------------
// UpdateMetricsReport
//------------------------------------------------------------------------------
bool StreamRecorder::UpdateMetricsReport() {
  auto num_dropped_spans = num_dropped_spans_.exchange(0);
  if (num_dropped_spans == 0) {
    return false;
  }
  collector::InternalMetrics report;
  auto count = report.add_counts();
  count->set_name("spans.dropped");
  count->set_int_value(num_dropped_spans);
  if (!message_buffer_.Add(PacketType::Metrics, report)) {
    // Add the dropped spans back since we failed to add the report.
    num_dropped_spans_ += num_dropped_spans;
  }
  return true;
}
}  // namespace lightstep

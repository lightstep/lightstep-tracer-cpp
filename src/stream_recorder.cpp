#include "stream_recorder.h"

#include "lightstep-tracer-common/collector.pb.h"
#include "utility.h"

#include <chrono>
#include <stdexcept>

namespace lightstep {
static const std::chrono::steady_clock::duration polling_interval =
    std::chrono::milliseconds{100};

//------------------------------------------------------------------------------
// MakeStreamInitializationMessage
//------------------------------------------------------------------------------
static collector::StreamInitialization MakeStreamInitializationMessage(
    const LightStepTracerOptions& options) {
  collector::StreamInitialization initialization;
  initialization.set_reporter_id(GenerateId());
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
      transporter_{std::move(transporter)},
      message_buffer_{options.message_buffer_size} {
  notification_threshold_ =
      static_cast<size_t>(options.message_buffer_size * 0.10);
  streamer_thread_ = std::thread{&StreamRecorder::RunStreamer, this};
  if (!message_buffer_.Add(MakeStreamInitializationMessage(options))) {
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
  message_buffer_.Add(span);

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
    while (1) {
      if (!message_buffer_.Consume(StreamRecorder::Consume,
                                   static_cast<void*>(this))) {
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
  condition_variable_.wait_for(lock, polling_interval, [this] {
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
}  // namespace lightstep

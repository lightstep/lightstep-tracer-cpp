#include "streaming_recorder.h"

#include <chrono>

namespace lightstep {
static const std::chrono::steady_clock::duration polling_interval =
    std::chrono::microseconds{10};

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamingRecorder::StreamingRecorder(Logger& logger,
                                     LightStepTracerOptions&& options,
                                     std::unique_ptr<StreamTransporter>&& transporter) 
  : logger_{logger},
    span_buffer_{options.span_buffer_size},
    transporter_{std::move(transporter)}
{
  streamer_thread_ = std::thread{&StreamingRecorder::RunStreamer, this};
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamingRecorder::~StreamingRecorder() {
  MakeStreamerExit();
  streamer_thread_.join();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void StreamingRecorder::RecordSpan(collector::Span&& span) noexcept {
  span_buffer_.Add(span);   
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool StreamingRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {}

//------------------------------------------------------------------------------
// RunStreamer
//------------------------------------------------------------------------------
void StreamingRecorder::RunStreamer() noexcept try {
  while (SleepForNextPoll()) {
    while (1) {
      if (!span_buffer_.Consume(StreamingRecorder::Consume,
                                static_cast<void*>(this))) {
        break;
      }
    }
  }
} catch (const std::exception& /*e*/) {
  MakeStreamerExit();
}

//------------------------------------------------------------------------------
// MakeStreamerExit
//------------------------------------------------------------------------------
void StreamingRecorder::MakeStreamerExit() noexcept {
  {
    std::unique_lock<std::mutex> lock{mutex_};
    exit_streamer_ = true;
  }
  condition_variable_.notify_all();
}

//------------------------------------------------------------------------------
// StreamingRecorder
//------------------------------------------------------------------------------
bool StreamingRecorder::SleepForNextPoll() {
  std::unique_lock<std::mutex> lock{mutex_};
  return !condition_variable_.wait_for(lock, polling_interval,
                                       [this] { return this->exit_streamer_; });
}

//------------------------------------------------------------------------------
// Consume
//------------------------------------------------------------------------------
size_t StreamingRecorder::Consume(void* context, const char* data,
                                  size_t num_bytes) {
  auto& stream_recorder = *static_cast<StreamingRecorder*>(context);
  return stream_recorder.transporter_->Write(data, num_bytes);
}
}  // namespace lightstep

#include "streaming_recorder.h"

#include <chrono>

namespace lightstep {
static const std::chrono::steady_clock::duration polling_interval =
    std::chrono::milliseconds{1};

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamingRecorder::StreamingRecorder(Logger& logger,
                                     LightStepTracerOptions&& options) 
  : logger_{logger}
{
  streamer_thread_ = std::thread{&StreamingRecorder::RunStreamer, this};
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
StreamingRecorder::~StreamingRecorder() {
  MakeStreamerExit();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void StreamingRecorder::RecordSpan(collector::Span&& span) noexcept {}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool StreamingRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {}

//------------------------------------------------------------------------------
// RunStreamer
//------------------------------------------------------------------------------
void StreamingRecorder::RunStreamer() noexcept {
  while (SleepForNextPoll()) {
  }
}

//------------------------------------------------------------------------------
// MakeStreamerExit
//------------------------------------------------------------------------------
void StreamingRecorder::MakeStreamerExit() noexcept {
  std::unique_lock<std::mutex> lock{mutex_};
  exit_streamer_ = true;
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
}  // namespace lightstep

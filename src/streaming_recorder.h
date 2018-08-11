#pragma once

#include "recorder.h"
#include "logger.h"
#include "span_buffer.h"

#include <atomic>
#include <thread>
#include <condition_variable>

namespace lightstep {
class StreamingRecorder : public Recorder {
 public:
  StreamingRecorder(Logger& logger, LightStepTracerOptions&& options);

  ~StreamingRecorder();

  void RecordSpan(collector::Span&& span) noexcept final;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept final;

 private:
  Logger& logger_;
  std::thread streamer_thread_;
  std::mutex mutex_;
  std::condition_variable condition_variable_;

  bool exit_streamer_{false};

  void RunStreamer() noexcept;

  void MakeStreamerExit() noexcept;

  bool SleepForNextPoll();
};
} // namespace lightstep

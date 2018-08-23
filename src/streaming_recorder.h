#pragma once

#include "logger.h"
#include "message_buffer.h"
#include "recorder.h"

#include <atomic>
#include <condition_variable>
#include <thread>

namespace lightstep {
class StreamingRecorder final : public Recorder {
 public:
  StreamingRecorder(Logger& logger, LightStepTracerOptions&& options,
                    std::unique_ptr<StreamTransporter>&& transporter);

  ~StreamingRecorder();

  void RecordSpan(const collector::Span& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  Logger& logger_;
  std::unique_ptr<StreamTransporter> transporter_;
  std::thread streamer_thread_;
  std::mutex mutex_;
  std::condition_variable condition_variable_;
  MessageBuffer message_buffer_;

  bool exit_streamer_{false};

  void RunStreamer() noexcept;

  void MakeStreamerExit() noexcept;

  bool SleepForNextPoll();

  static size_t Consume(void* context, const char* data, size_t num_bytes);
};
}  // namespace lightstep

#pragma once

#include "condition_variable.h"
#include "logger.h"
#include "message_buffer.h"
#include "recorder.h"

#include <atomic>
#include <condition_variable>
#include <thread>

namespace lightstep {
class StreamRecorder final : public Recorder {
 public:
  StreamRecorder(Logger& logger, LightStepTracerOptions&& options,
                 std::unique_ptr<StreamTransporter>&& transporter);

  ~StreamRecorder() override;

  void RecordSpan(const collector::Span& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions options_;
  size_t notification_threshold_;
  std::unique_ptr<StreamTransporter> transporter_;
  std::thread streamer_thread_;

  ConditionVariable condition_variable_;
  MessageBuffer message_buffer_;

  std::atomic<bool> waiting_{false};

  std::atomic<size_t> num_dropped_spans_{0};

  void RunStreamer() noexcept;

  void MakeStreamerExit() noexcept;

  bool SleepForNextPoll();

  static size_t Consume(void* context, const char* data, size_t num_bytes);

  bool UpdateMetricsReport();
};
}  // namespace lightstep

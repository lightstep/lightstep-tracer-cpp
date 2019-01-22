#pragma once

#include <atomic>
#include <thread>

#include "common/chunk_circular_buffer.h"
#include "common/logger.h"
#include "lightstep/tracer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * A Recorder that load balances and streams spans to multiple satellites.
 */
class StreamRecorder final : public Recorder {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  ~StreamRecorder() noexcept;

  // Recorder
  void RecordSpan(const collector::Span& span) noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
  ChunkCircularBuffer span_buffer_;
  size_t early_flush_marker_;

  EventBase event_base_;
  TimerEvent poll_timer_;
  TimerEvent flush_timer_;

  std::thread thread_;
  std::atomic<bool> exit_{false};

  void Run() noexcept;

  void Poll() noexcept;

  void Flush() noexcept;
};
}  // namespace lightstep

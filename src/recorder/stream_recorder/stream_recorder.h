#pragma once

#include <atomic>
#include <thread>

#include "lightstep/tracer.h"
#include "common/chunk_circular_buffer.h"
#include "common/logger.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
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

  EventBase event_base_;
  TimerEvent poll_timer_;

  std::thread thread_;
  std::atomic<bool> exit_{false};

  void Run() noexcept;

  void Poll() noexcept;

  template <void (StreamRecorder::*MemberFunction)()>
  static EventBase::EventCallback MakeTimerCallback() {
    return [] (int /*socket*/, short /*what*/, void* context) {
      (static_cast<StreamRecorder*>(context)->*MemberFunction)();
    };
  }
};
} // namespace lightstep

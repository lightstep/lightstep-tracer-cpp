#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include "common/chunk_circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/tracer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/satellite_streamer.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * A Recorder that load balances and streams spans to multiple satellites.
 */
class StreamRecorder final : public Recorder, private Noncopyable {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  ~StreamRecorder() noexcept override;

  /**
   * @return true if no spans are buffered in the recorder.
   */
  bool empty() const noexcept { return span_buffer_.buffer().empty(); }

  // Recorder
  void RecordSpan(const collector::Span& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
  StreamRecorderMetrics metrics_;
  ChunkCircularBuffer span_buffer_;
  size_t early_flush_marker_;

  EventBase event_base_;
  TimerEvent poll_timer_;
  TimerEvent flush_timer_;

  SatelliteStreamer streamer_;

  std::thread thread_;
  std::atomic<bool> exit_{false};

  std::mutex flush_mutex_;
  std::condition_variable flush_condition_variable_;
  std::atomic<int> flush_counter_{0};
  uint64_t num_bytes_consumed_{0};

  void Run() noexcept;

  void Poll() noexcept;

  void Flush() noexcept;
};
}  // namespace lightstep

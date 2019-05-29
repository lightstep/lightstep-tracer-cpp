#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include "stream_recorder_impl.h"

#include "common/chunk_circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "lightstep/tracer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder.h"
#include "recorder/threaded_recorder.h"
#include "recorder/stream_recorder/satellite_streamer.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * A Recorder that load balances and streams spans to multiple satellites.
 */
class StreamRecorder final : public ThreadedRecorder, private Noncopyable {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  ~StreamRecorder() noexcept override;

  /**
   * @return true if no spans are buffered in the recorder.
   */
  bool empty() const noexcept { return span_buffer_.buffer().empty(); }

  Logger& logger() const noexcept { return logger_; }

  const LightStepTracerOptions& tracer_options() const noexcept {
    return tracer_options_;
  }

  const StreamRecorderOptions& recorder_options() const noexcept {
    return recorder_options_;
  }

  StreamRecorderMetrics& metrics() noexcept { return metrics_; }

  ChunkCircularBuffer& span_buffer() noexcept { return span_buffer_; }

  void Poll() noexcept;

  int ConsumePendingFlushCount() noexcept {
    return pending_flush_counter_.exchange(0);
  }

  // ThreadedRecorder
  void RecordSpan(const collector::Span& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  void PrepareForFork() noexcept override;

  void OnForkedParent() noexcept override;

  void OnForkedChild() noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
  StreamRecorderMetrics metrics_;
  ChunkCircularBuffer span_buffer_;

  std::atomic<bool> exit_{false};

  std::mutex flush_mutex_;
  std::condition_variable flush_condition_variable_;
  std::atomic<int> pending_flush_counter_{0};
  uint64_t num_bytes_consumed_{0};

  std::unique_ptr<StreamRecorderImpl> stream_recorder_impl_;
};
}  // namespace lightstep

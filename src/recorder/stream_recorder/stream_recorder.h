#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include "stream_recorder_impl.h"

#include "common/circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "common/serialization_chain.h"
#include "lightstep/tracer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/fork_aware_recorder.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * TODO(rnburn): this will replace StreamRecorder when the implementation is
 * finished.
 */
class StreamRecorder : public ForkAwareRecorder, private Noncopyable {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  ~StreamRecorder() noexcept override;

  /**
   * Checks whether any threads blocked on flush calls can be resumed.
   */
  void Poll() noexcept;

  /**
   * Gets the pending flush count and resets the counter.
   * @return the pending flush count.
   */
  int ConsumePendingFlushCount() noexcept {
    return pending_flush_counter_.exchange(0);
  }

  /**
   * @return true if no spans are buffered in the recorder.
   */
  bool empty() const noexcept { return span_buffer_.empty(); }

  /**
   * @return the associated Logger.
   */
  Logger& logger() const noexcept { return logger_; }

  /**
   * @return the associated LightStepTracerOptions
   */
  const LightStepTracerOptions& tracer_options() const noexcept {
    return tracer_options_;
  }

  /**
   * @return the associated StreamRecorderOptions.
   */
  const StreamRecorderOptions& recorder_options() const noexcept {
    return recorder_options_;
  }

  /**
   * @return the associated StreamRecorderMetrics.
   */
  StreamRecorderMetrics& metrics() noexcept { return metrics_; }

  /**
   * @return the associated span buffer.
   */
  CircularBuffer<SerializationChain>& span_buffer() noexcept {
    return span_buffer_;
  }

  // Recorder
  void RecordSpan(std::unique_ptr<SerializationChain>&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  // ForkAwareRecorder
  void PrepareForFork() noexcept override;

  void OnForkedParent() noexcept override;

  void OnForkedChild() noexcept override;

 private:
  Logger& logger_;
  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
  StreamRecorderMetrics metrics_;
  CircularBuffer<SerializationChain> span_buffer_;

  std::atomic<bool> exit_{false};

  std::mutex flush_mutex_;
  std::condition_variable flush_condition_variable_;
  std::atomic<int> pending_flush_counter_{0};
  int64_t num_spans_consumed_{0};

  std::unique_ptr<StreamRecorderImpl> stream_recorder_impl_;
};
}  // namespace lightstep

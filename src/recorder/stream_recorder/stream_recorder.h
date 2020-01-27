#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include "stream_recorder_impl.h"

#include "common/chained_stream.h"
#include "common/circular_buffer.h"
#include "common/logger.h"
#include "common/noncopyable.h"
#include "common/platform/network_environment.h"
#include "lightstep/tracer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/fork_aware_recorder.h"
#include "recorder/metrics_tracker.h"
#include "recorder/stream_recorder.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
/**
 * A Recorder that load balances and streams spans to multiple satellites.
 */
class StreamRecorder : public ForkAwareRecorder, private Noncopyable {
 public:
  StreamRecorder(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

  ~StreamRecorder() noexcept override;

  /**
   * Checks whether any threads blocked on flush calls can be resumed.
   * @param stream_recorder_impl a reference to the stream recorder
   * implementation that invoked poll.
   *
   * Note: stream_recorder_impl is redundant since it can also be accessed
   * through the member variable stream_recorder_impl_, but it's structured this
   * way to avoid false positives from TSAN.
   */
  void Poll(StreamRecorderImpl& stream_recorder_impl) noexcept;

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
   * @return the associated MetricsTracker.
   */
  MetricsTracker& metrics() noexcept { return metrics_; }

  /**
   * @return the associated span buffer.
   */
  CircularBuffer<ChainedStream>& span_buffer() noexcept { return span_buffer_; }

  // Recorder
  Fragment ReserveHeaderSpace(ChainedStream& stream) override;

  void WriteFooter(
      google::protobuf::io::CodedOutputStream& coded_stream) override;

  void RecordSpan(Fragment header_fragment,
                  std::unique_ptr<ChainedStream>&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  bool ShutdownWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  int64_t ComputeSystemSteadyTimestampDelta() const noexcept override {
    return stream_recorder_impl_->timestamp_delta();
  }

  std::chrono::system_clock::time_point ComputeCurrentSystemTimestamp(
      std::chrono::steady_clock::time_point steady_now) const
      noexcept override {
    return ToSystemTimestamp(stream_recorder_impl_->timestamp_delta(),
                             steady_now);
  }

  const MetricsObserver* metrics_observer() const noexcept override {
    return tracer_options_.metrics_observer.get();
  }

  // ForkAwareRecorder
  void PrepareForFork() noexcept override;

  void OnForkedParent() noexcept override;

  void OnForkedChild() noexcept override;

 private:
  Logger& logger_;
  NetworkEnvironment network_environment_;

  LightStepTracerOptions tracer_options_;
  StreamRecorderOptions recorder_options_;
  MetricsTracker metrics_;
  CircularBuffer<ChainedStream> span_buffer_;

  std::atomic<bool> exit_{false};

  std::mutex flush_mutex_;
  std::condition_variable flush_condition_variable_;
  std::atomic<int> pending_flush_counter_{0};
  int64_t num_spans_consumed_{0};

  std::mutex shutdown_mutex_;
  std::condition_variable shutdown_condition_variable_;
  std::atomic<int> shutdown_counter_{0};

  // Used by polling to track when the satellite connections become inactive so
  // that any threads waiting on shutdown can be notified.
  bool last_is_active_{true};

  std::unique_ptr<StreamRecorderImpl> stream_recorder_impl_;
};
}  // namespace lightstep

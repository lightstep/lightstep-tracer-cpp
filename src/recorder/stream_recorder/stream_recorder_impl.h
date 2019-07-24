#pragma once

#include <atomic>
#include <thread>

#include "common/noncopyable.h"
#include "common/timestamp.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder/satellite_streamer.h"

namespace lightstep {
class StreamRecorder;

/**
 * Implements the part of StreamRecorder that manages the recording thread and
 * satellite connections.
 *
 * This functionality is broken out into a separate class so that the resources
 * can be brought down and resumed so as to support forking.
 */
class StreamRecorderImpl : private Noncopyable {
 public:
  explicit StreamRecorderImpl(StreamRecorder& stream_recorder);

  ~StreamRecorderImpl() noexcept;

  int64_t timestamp_delta() const noexcept { return timestamp_delta_; }

 private:
  StreamRecorder& stream_recorder_;

  EventBase event_base_;
  size_t early_flush_marker_;
  TimerEvent poll_timer_;
  TimerEvent timestamp_delta_timer_;
  TimerEvent flush_timer_;

  SatelliteStreamer streamer_;

  std::thread thread_;
  std::atomic<bool> exit_{false};
  std::atomic<int64_t> timestamp_delta_{ComputeSystemSteadyTimestampDelta()};

  void Run() noexcept;

  void Poll() noexcept;

  void ComputeTimestampDelta() noexcept;

  void Flush() noexcept;
};
}  // namespace lightstep

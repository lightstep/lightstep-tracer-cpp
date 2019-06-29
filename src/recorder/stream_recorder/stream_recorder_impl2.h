#pragma once

#include <atomic>
#include <thread>

#include "common/noncopyable.h"
#include "network/event_base.h"
#include "network/timer_event.h"

namespace lightstep {
class StreamRecorder2;

/**
 * Implements the part of StreamRecorder that manages the recording thread and
 * satellite connections.
 *
 * This functionality is broken out into a separate class so that the resources
 * can be brought down and resumed so as to support forking.
 */
class StreamRecorderImpl2 : private Noncopyable {
 public:
  explicit StreamRecorderImpl2(StreamRecorder2& stream_recorder);

  ~StreamRecorderImpl2() noexcept;

 private:
  StreamRecorder2& stream_recorder_;

  EventBase event_base_;
  size_t early_flush_marker_;
  TimerEvent poll_timer_;
  TimerEvent flush_timer_;

  /* SatelliteStreamer streamer_; */

  std::thread thread_;
  std::atomic<bool> exit_{false};

  void Run() noexcept;

  void Poll() noexcept;

  void Flush() noexcept;
};
}  // namespace lightstep

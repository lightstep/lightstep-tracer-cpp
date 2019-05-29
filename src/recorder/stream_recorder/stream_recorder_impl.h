#pragma once

#include <atomic>
#include <thread>

#include "recorder/stream_recorder/satellite_streamer.h"
#include "network/event_base.h"
#include "network/timer_event.h"
#include "common/noncopyable.h"

namespace lightstep {
class StreamRecorder;

class StreamRecorderImpl : private Noncopyable {
 public:
   explicit StreamRecorderImpl(StreamRecorder& stream_recorder);

   ~StreamRecorderImpl() noexcept;

 private:
  StreamRecorder& stream_recorder_;

  EventBase event_base_;
  size_t early_flush_marker_;
  TimerEvent poll_timer_;
  TimerEvent flush_timer_;

  SatelliteStreamer streamer_;

  std::thread thread_;
  std::atomic<bool> exit_{false};

  void Run() noexcept;

  void Poll() noexcept;

  void Flush() noexcept;
};
} // namespace lightstep

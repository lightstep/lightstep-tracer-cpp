#pragma once

#include "common/noncopyable.h"
#include "network/event.h"
#include "network/socket.h"

namespace lightstep {
class SatelliteStreamer;

class SatelliteConnection : private Noncopyable {
 public:
  explicit SatelliteConnection(SatelliteStreamer& streamer);

  /**
   * Start establishing a connection to a satellite.
   */
  void Start() noexcept;

 private:
  SatelliteStreamer& streamer_;
  Socket socket_{-1};
  Event read_event_;
  Event write_event_;
  Event reconnect_timer_;

  void Connect() noexcept;

  void FreeSocket();

  void HandleFailure() noexcept;

  void ScheduleReconnect();

  void Reconnect() noexcept;

  void OnReadable(int file_descriptor, short what) noexcept;

  void OnWritable(int file_descriptor, short what) noexcept;

  void OnSocketError() noexcept;
};
}  // namespace lightstep

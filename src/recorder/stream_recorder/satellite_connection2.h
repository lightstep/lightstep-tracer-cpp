#pragma once

#include "common/noncopyable.h"
#include "network/event.h"
#include "network/socket.h"
#include "recorder/stream_recorder/connection_stream2.h"
#include "recorder/stream_recorder/host_header.h"
#include "recorder/stream_recorder/status_line_parser.h"

namespace lightstep {
class SatelliteStreamer2;

/**
 * Manages a connection to an individual statellite.
 */
class SatelliteConnection2 : private Noncopyable {
 public:
  explicit SatelliteConnection2(SatelliteStreamer2& streamer);

  ~SatelliteConnection2() noexcept;

  /**
   * Start establishing a connection to a satellite.
   */
  void Start() noexcept;

  /**
   * Flushes through the streaming satellite connection.
   * @return true if all data in th span buffer was flushed through the
   * connection.
   */
  bool Flush() noexcept;

  /**
   * @return true if the satellite connection is available for streaming spans.
   */
  bool ready() const noexcept;

 private:
  SatelliteStreamer2& streamer_;
  HostHeader host_header_;
  ConnectionStream2 connection_stream_;
  StatusLineParser status_line_parser_;
  Socket socket_{-1};
  bool writable_{false};
  Event read_event_;
  Event write_event_;
  Event reconnect_timer_;
  Event graceful_shutdown_timeout_;

  void Connect() noexcept;

  void FreeSocket();

  void HandleFailure() noexcept;

  void ScheduleReconnect();

  void InitiateReconnect() noexcept;

  void Reconnect() noexcept;

  void GracefulShutdownTimeout() noexcept;

  void OnReadable(int file_descriptor, short what) noexcept;

  void OnWritable(int file_descriptor, short what) noexcept;

  void OnSocketError() noexcept;
};
}  // namespace lightstep

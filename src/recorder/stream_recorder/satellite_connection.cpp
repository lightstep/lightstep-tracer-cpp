#include "recorder/stream_recorder/satellite_connection.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <exception>

#include "common/random.h"
#include "network/timer_event.h"
#include "recorder/stream_recorder/satellite_streamer.h"

#include <event2/event.h>
#include <unistd.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(SatelliteStreamer& streamer)
    : streamer_{streamer},
      reconnect_timer_{streamer_.event_base(), -1, 0,
                       MakeTimerCallback<SatelliteConnection,
                                         &SatelliteConnection::Reconnect>(),
                       static_cast<void*>(this)} {}

//--------------------------------------------------------------------------------------------------
// Start
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::Start() noexcept { Connect(); }

//--------------------------------------------------------------------------------------------------
// Connect
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::Connect() noexcept try {
  auto endpoint = streamer_.endpoint_manager().RequestEndpoint();
  socket_ = lightstep::Connect(endpoint);
  ScheduleReconnect();

  read_event_ =
      Event{streamer_.event_base(), socket_.file_descriptor(), EV_READ,
            MakeEventCallback<SatelliteConnection,
                              &SatelliteConnection::OnReadable>(),
            static_cast<void*>(this)};
  read_event_.Add(nullptr);

  write_event_ =
      Event{streamer_.event_base(), socket_.file_descriptor(), EV_WRITE,
            MakeEventCallback<SatelliteConnection,
                              &SatelliteConnection::OnWritable>(),
            static_cast<void*>(this)};
  write_event_.Add(streamer_.recorder_options().satellite_write_timeout);
} catch (const std::exception& e) {
  streamer_.logger().Error("Connect failed: ", e.what());
  HandleFailure();
}

//--------------------------------------------------------------------------------------------------
// FreeSocket
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::FreeSocket() {
  socket_ = Socket{-1};
  read_event_ = Event{};
  write_event_ = Event{};
  reconnect_timer_.Remove();
}

//--------------------------------------------------------------------------------------------------
// HandleFailure
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::HandleFailure() noexcept try {
  FreeSocket();
  streamer_.event_base().OnTimeout(
      streamer_.recorder_options().satellite_failure_retry_period,
      MakeTimerCallback<SatelliteConnection, &SatelliteConnection::Connect>(),
      static_cast<void*>(this));
} catch (const std::exception& e) {
  streamer_.logger().Error("HandleFailure failed: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// ScheduleReconnect
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::ScheduleReconnect() {
  auto reconnect_period = GenerateRandomDuration(
      streamer_.recorder_options().min_satellite_reconnect_period,
      streamer_.recorder_options().max_satellite_reconnect_period);
  reconnect_timer_.Add(reconnect_period);
}

//--------------------------------------------------------------------------------------------------
// Reconnect
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::Reconnect() noexcept try {
  FreeSocket();
  Connect();
} catch (const std::exception& e) {
  streamer_.logger().Error("Reconnect failed: ", e.what());
}

//--------------------------------------------------------------------------------------------------
// OnReadable
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::OnReadable(int file_descriptor,
                                     short /*what*/) noexcept try {
  std::array<char, 512> buffer;
  ssize_t rcode;

  // Ignore the contents of any response
  while (1) {
    rcode = ::read(file_descriptor, static_cast<void*>(buffer.data()),
                   buffer.size());
    if (rcode <= 0) {
      break;
    }
  }

  if (rcode == 0) {
    streamer_.logger().Warn("Socket closed by satellite");
    return OnSocketError();
  }
  assert(rcode == -1);
  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    return read_event_.Add(nullptr);
  }
  streamer_.logger().Error("Satellite socket error: ", std::strerror(errno));
  return OnSocketError();
} catch (const std::exception& e) {
  streamer_.logger().Error("OnReadable failed: ", e.what());
  return HandleFailure();
}

//--------------------------------------------------------------------------------------------------
// OnWritable
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::OnWritable(int /*file_descriptor*/,
                                     short what) noexcept try {
  if ((what & EV_TIMEOUT) != 0) {
    streamer_.logger().Error("Satellite connection timed out");
    return OnSocketError();
  }
  assert((what & EV_WRITE) != 0);
  streamer_.OnConnectionWritable(this);
} catch (const std::exception& e) {
  streamer_.logger().Error("OnWritable failed: ", e.what());
  return HandleFailure();
}

//--------------------------------------------------------------------------------------------------
// OnSocketError
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::OnSocketError() noexcept try {
  FreeSocket();
  Connect();
} catch (const std::exception& e) {
  streamer_.logger().Error("OnSocketError failed: ", e.what());
  return HandleFailure();
}
}  // namespace lightstep

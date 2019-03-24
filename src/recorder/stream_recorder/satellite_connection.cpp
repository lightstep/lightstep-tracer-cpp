#include "recorder/stream_recorder/satellite_connection.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <exception>

#include "common/random.h"
#include "network/timer_event.h"
#include "network/vector_write.h"
#include "recorder/stream_recorder/satellite_streamer.h"

#include <event2/event.h>
#include <unistd.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(SatelliteStreamer& streamer)
    : streamer_{streamer},
      host_header_{streamer.tracer_options()},
      connection_stream_{host_header_.fragment(),
                         streamer.header_common_fragment(), streamer.metrics(),
                         streamer.span_stream()},
      reconnect_timer_{
          streamer_.event_base(), -1, 0,
          MakeTimerCallback<SatelliteConnection,
                            &SatelliteConnection::InitiateReconnect>(),
          static_cast<void*>(this)},
      graceful_shutdown_timeout_{
          streamer.event_base(), -1, 0,
          MakeTimerCallback<SatelliteConnection,
                            &SatelliteConnection::GracefulShutdownTimeout>(),
          static_cast<void*>(this)} {}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::~SatelliteConnection() noexcept {
  // If there's a streaming session open, attempt to cleanly close it if we can
  // do so without blocking.
  if (writable_) {
    connection_stream_.Shutdown();
    Flush();
  }
}

//--------------------------------------------------------------------------------------------------
// Start
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::Start() noexcept { Connect(); }

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
bool SatelliteConnection::Flush() noexcept try {
  auto flushed_everything = connection_stream_.Flush(
      [this](std::initializer_list<FragmentInputStream*> fragment_streams) {
        return Write(socket_.file_descriptor(), fragment_streams);
      });
  if (flushed_everything) {
    writable_ = true;
  } else {
    writable_ = false;
    write_event_.Add(streamer_.recorder_options().satellite_write_timeout);
  }
  return flushed_everything;
} catch (const std::exception& e) {
  streamer_.logger().Error("Flush failed: ", e.what());
  HandleFailure();
  return false;
}

//--------------------------------------------------------------------------------------------------
// ready
//--------------------------------------------------------------------------------------------------
bool SatelliteConnection::ready() const noexcept {
  return writable_ && !connection_stream_.shutting_down();
}

//--------------------------------------------------------------------------------------------------
// Connect
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::Connect() noexcept try {
  auto endpoint = streamer_.endpoint_manager().RequestEndpoint();
  socket_ = lightstep::Connect(endpoint.first);
  host_header_.set_host(endpoint.second);
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
  graceful_shutdown_timeout_.Remove();
  writable_ = false;
  connection_stream_.Reset();
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
// InitiateReconnect
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::InitiateReconnect() noexcept try {
  connection_stream_.Shutdown();
  if (writable_) {
    Flush();
  }
  graceful_shutdown_timeout_.Add(
      streamer_.recorder_options().satellite_graceful_stream_shutdown_timeout);
} catch (const std::exception& e) {
  streamer_.logger().Error("InitiateReconnect failed: ", e.what());
  return HandleFailure();
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
// GracefulShutdownTimeout
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::GracefulShutdownTimeout() noexcept {
  streamer_.logger().Error(
      "Failed to shutdown satellite connection gracefully");
  Reconnect();
}

//--------------------------------------------------------------------------------------------------
// OnReadable
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::OnReadable(int file_descriptor,
                                     short /*what*/) noexcept try {
  std::array<char, 512> buffer;
  ssize_t rcode;

  // Ignore the contents of any response
  while (true) {
    rcode = ::read(file_descriptor, static_cast<void*>(buffer.data()),
                   buffer.size());
    if (rcode <= 0) {
      break;
    }
  }

  if (rcode == 0) {
    if (!connection_stream_.completed()) {
      streamer_.logger().Warn("Socket closed prematurely by satellite");
      return OnSocketError();
    }
    return Reconnect();
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
  Flush();
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

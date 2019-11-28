#include "recorder/stream_recorder/satellite_connection.h"

#include <array>
#include <cassert>
#include <exception>

#include "common/platform/error.h"
#include "common/random.h"
#include "network/timer_event.h"
#include "network/vector_write.h"
#include "recorder/stream_recorder/satellite_streamer.h"

#include <event2/event.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnection::SatelliteConnection(SatelliteStreamer& streamer)
    : streamer_{streamer},
      host_header_{streamer.tracer_options()},
      connection_stream_{host_header_.fragment(),
                         streamer.header_common_fragment(),
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
    streamer_.logger().Info("Flushed everything to file_descriptor ",
                            socket_.file_descriptor());
  } else {
    writable_ = false;
    write_event_.Add(streamer_.recorder_options().satellite_write_timeout);
    streamer_.logger().Info("Flushed partially to file_descriptor ",
                            socket_.file_descriptor());
  }
  return flushed_everything;
} catch (const std::exception& e) {
  streamer_.logger().Error("Flush failed: ", e.what());
  HandleFailure();
  return false;
}

//--------------------------------------------------------------------------------------------------
// InitiateShutdown
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::InitiateShutdown() noexcept {
  InitiateReconnect();
  is_shutting_down_ = true;
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
  streamer_.logger().Info("Connecting to satellite on ip ", endpoint.first);
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
  socket_ = Socket{InvalidSocket};
  read_event_ = Event{};
  write_event_ = Event{};
  reconnect_timer_.Remove();
  graceful_shutdown_timeout_.Remove();
  writable_ = false;
  connection_stream_.Reset();
  status_line_parser_.Reset();
}

//--------------------------------------------------------------------------------------------------
// HandleFailure
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::HandleFailure() noexcept try {
  FreeSocket();
  if (is_shutting_down_) {
    // if we failed during shutdown, don't try to reconnect
    was_shutdown_ = true;
    return;
  }
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
  if (is_shutting_down_) {
    // If we're shutting down, then we've already scheduled a reconnect so do
    // nothing.
    return;
  }
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
  if (is_shutting_down_) {
    // break the reconnection cycle
    was_shutdown_ = true;
    return;
  }
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
void SatelliteConnection::OnReadable(FileDescriptor file_descriptor,
                                     short /*what*/) noexcept try {
  streamer_.logger().Info("Satellite file_descriptor ", file_descriptor,
                          " is readable");
  std::array<char, 512> buffer;
  int rcode;

  // Read satellite response
  while (true) {
    rcode =
        Read(file_descriptor, static_cast<void*>(buffer.data()), buffer.size());
    if (rcode <= 0) {
      break;
    }
    status_line_parser_.Parse(
        opentracing::string_view{buffer.data(), static_cast<size_t>(rcode)});
  }

  if (rcode == 0) {
    if (!status_line_parser_.completed()) {
      streamer_.logger().Error("No status line from satellite response");
      return OnSocketError();
    }
    if (status_line_parser_.status_code() != 200) {
      streamer_.logger().Error("Error from satellite ",
                               status_line_parser_.status_code(), ": ",
                               status_line_parser_.reason());
      return OnSocketError();
    }
    if (!connection_stream_.completed()) {
      streamer_.logger().Warn("Socket closed prematurely by satellite");
      return OnSocketError();
    }
    return Reconnect();
  }
  assert(rcode < 0);
  auto error_code = GetLastErrorCode();
  if (IsBlockingErrorCode(error_code)) {
    return read_event_.Add(nullptr);
  }
  streamer_.logger().Error("Satellite socket error: ",
                           GetErrorCodeMessage(error_code));
  return OnSocketError();
} catch (const std::exception& e) {
  streamer_.logger().Error("OnReadable failed: ", e.what());
  return HandleFailure();
}

//--------------------------------------------------------------------------------------------------
// OnWritable
//--------------------------------------------------------------------------------------------------
void SatelliteConnection::OnWritable(FileDescriptor file_descriptor,
                                     short what) noexcept try {
  streamer_.logger().Info("Satellite file_descriptor ", file_descriptor,
                          " is writable");
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

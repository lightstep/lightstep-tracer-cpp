#include "satellite_stream_transporter.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SatelliteStreamTransporter::SatelliteStreamTransporter(
    Logger& logger, const LightStepTracerOptions& options)
    : satellite_connection_{logger, options.collector_host.c_str(),
                            static_cast<uint16_t>(options.collector_port)} {}

//------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------
size_t SatelliteStreamTransporter::Write(const char* buffer, size_t size) {
  if (!satellite_connection_.is_connected()) {
    // For now, fail the transport.
    //
    // This will be replaced with retry logic.
    throw std::runtime_error{"transport failed: no connection to satellite"};
  }
  auto result = write(satellite_connection_.file_descriptor(), buffer,
                      static_cast<int>(size));
  if (result < 0) {
    throw std::runtime_error{"failed to write to socket"};
  }
  return static_cast<size_t>(result);
}
}  // namespace lightstep

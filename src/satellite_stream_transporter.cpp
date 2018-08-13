#include "satellite_stream_transporter.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdexcept>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SatelliteStreamTransporter::Socket::Socket() {
  file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (file_descriptor < 0) {
    throw std::runtime_error{"failed to create socket"};
  }
}

SatelliteStreamTransporter::Socket::~Socket() { close(file_descriptor); }

SatelliteStreamTransporter::SatelliteStreamTransporter(
    const LightStepTracerOptions& options) {
  sockaddr_in satellite_address = {};
  satellite_address.sin_family = AF_INET;
  satellite_address.sin_port = htons(options.collector_port);

  auto rcode = inet_pton(AF_INET, options.collector_host.c_str(),
                         &satellite_address.sin_addr);
  if (rcode <= 0) {
    throw std::runtime_error{"inet_pton failed"};
  }

  rcode = connect(socket_.file_descriptor,
                  reinterpret_cast<sockaddr*>(&satellite_address),
                  sizeof(satellite_address));
  if (rcode < 0) {
    throw std::runtime_error{"failed to connect to satellite"};
  }
}

//------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------
size_t SatelliteStreamTransporter::Write(const char* buffer, size_t size) {
  auto result = write(socket_.file_descriptor, buffer, static_cast<int>(size));
  if (result < 0) {
    throw std::runtime_error{"failed to write to socket"};
  }
  return static_cast<size_t>(result);
}
}  // namespace lightstep

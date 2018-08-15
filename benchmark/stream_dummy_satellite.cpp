#include "stream_dummy_satellite.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <stdexcept>

struct Socket {
  Socket() {
    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor < 0) {
      throw std::runtime_error{"failed to create socket"};
    }
  }

  explicit Socket(int fd) : file_descriptor{fd} {}

  ~Socket() {
    close(file_descriptor);
  }

  int file_descriptor;
};

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
StreamDummySatellite::StreamDummySatellite(const char* host, int port)
    : host_{host}, port_{port} {}

//------------------------------------------------------------------------------
// Run
//------------------------------------------------------------------------------
void StreamDummySatellite::Run() {
  sockaddr_in satellite_address = {};
  satellite_address.sin_family = AF_INET;
  satellite_address.sin_port = htons(port_);

  auto rcode = inet_pton(AF_INET, host_.c_str(), &satellite_address.sin_addr);
  if (rcode <= 0) {
    throw std::runtime_error{"inet_pton failed"};
  }

  Socket listen_socket;
  rcode = bind(listen_socket.file_descriptor,
               reinterpret_cast<sockaddr*>(&satellite_address),
               sizeof(satellite_address));
  if (rcode != 0) {
    throw std::runtime_error{"bind failed"};
  }

  /* rcode = fcntl(listen_socket.file_descriptor, F_SETFL, */
  /*               fcntl(socketfd, F_GETFL, 0) | O_NONBLOCK); */

  rcode = listen(listen_socket.file_descriptor, 10);
  if (rcode != 0) {
    throw std::runtime_error{"listen failed"};
  }

}

//------------------------------------------------------------------------------
// ProcessConnections
//------------------------------------------------------------------------------
void StreamDummySatellite::ProcessConnections() {
}

//------------------------------------------------------------------------------
// span_ids
//------------------------------------------------------------------------------
std::vector<uint64_t> StreamDummySatellite::span_ids() const {
  return {};
}

//------------------------------------------------------------------------------
// Reserve
//------------------------------------------------------------------------------
void StreamDummySatellite::Reserve(size_t num_span_ids) {
}
}  // namespace lightstep

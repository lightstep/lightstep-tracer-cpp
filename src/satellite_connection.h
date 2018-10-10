#pragma once

#include <cstdint>

#include "logger.h"
#include "socket.h"

namespace lightstep {
class SatelliteConnection {
 public:
  SatelliteConnection(Logger& logger, const char* host, uint16_t port);

  bool reconnect() noexcept;

  bool is_connected() const noexcept { return is_connected_; }

  int file_descriptor() const noexcept { return socket_.file_descriptor(); }

 private:
  bool connect() noexcept;

  Logger& logger_;
  Socket socket_;
  std::string host_;
  bool is_connected_{false};
  uint16_t port_;
};
}  // namespace lightstep

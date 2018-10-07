#pragma once

#include <cstdint>

namespace lightstep {
class TcpConnection {
 public:
  TcpConnection(const char* host, uint16_t port);
};
}  // namespace lightstep

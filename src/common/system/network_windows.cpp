#include "common/system/network.h"

namespace lightstep {
int Read(int socket, void* data, size_t size) noexcept {
  return ::recv(socket, static_cast<char*>(data), static_cast<int>(size), 0);
}
} // namespace lightstep

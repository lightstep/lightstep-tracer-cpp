#include "common/system/network.h"

#include <unistd.h>

namespace lightstep {
int Read(int socket, void* data, size_t size) noexcept {
  return static_cast<int>(::read(socket, data, size));
}
} // namespace lightstep

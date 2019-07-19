#include "common/system/error.h"

#include <cerrno>

namespace lightstep {
ErrorCode GetLastError() noexcept {
  return errno;
}
} // namespace lightstep

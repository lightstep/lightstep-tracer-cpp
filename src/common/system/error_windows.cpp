#include "common/system/error.h"

#include <errhandlingapi.h>

namespace lightstep {
ErrorCode GetLastError() noexcept {
  return ::GetLastError();
}
} // namespace lightstep

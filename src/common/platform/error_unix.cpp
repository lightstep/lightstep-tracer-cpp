#include "common/platform/error.h"

#include <cerrno>
#include <cstring>

#include <unistd.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GetLastErrorCode
//--------------------------------------------------------------------------------------------------
ErrorCode GetLastErrorCode() noexcept { return errno; }

//--------------------------------------------------------------------------------------------------
// GetErrorCodeMessage
//--------------------------------------------------------------------------------------------------
std::string GetErrorCodeMessage(ErrorCode error_code) {
  return std::strerror(error_code);
}

//--------------------------------------------------------------------------------------------------
// IsBlockingErrorCode
//--------------------------------------------------------------------------------------------------
bool IsBlockingErrorCode(ErrorCode error_code) noexcept {
  return error_code == EAGAIN || error_code == EWOULDBLOCK;
}

//--------------------------------------------------------------------------------------------------
// IsInProgressErrorCode
//--------------------------------------------------------------------------------------------------
bool IsInProgressErrorCode(ErrorCode error_code) noexcept {
  return error_code == EINPROGRESS;
}
}  // namespace lightstep

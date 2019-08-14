#include "common/platform/error.h"

#include <errhandlingapi.h>
#include <winbase.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GetLastErrorCode
//--------------------------------------------------------------------------------------------------
ErrorCode GetLastErrorCode() noexcept { return GetLastError(); }

//--------------------------------------------------------------------------------------------------
// GetErrorCodeMessage
//--------------------------------------------------------------------------------------------------
std::string GetErrorCodeMessage(ErrorCode error_code) {
  // See https://stackoverflow.com/a/455533/4447365
  LPTSTR error_text = nullptr;
  auto size = FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPTSTR>(&error_text), 0, nullptr);
  if (error_text == nullptr) {
    return {};
  }
  try {
    std::string result{error_text, size};
    LocalFree(error_text);
    return result;
  } catch (...) {
    LocalFree(error_text);
    throw;
  }
}

//--------------------------------------------------------------------------------------------------
// IsBlockingErrorCode
//--------------------------------------------------------------------------------------------------
bool IsBlockingErrorCode(ErrorCode error_code) noexcept {
  return error_code == WSAEWOULDBLOCK;
}

//--------------------------------------------------------------------------------------------------
// IsInProgressErrorCode
//--------------------------------------------------------------------------------------------------
bool IsInProgressErrorCode(ErrorCode error_code) noexcept {
  return error_code == WSAEINPROGRESS;
}
}  // namespace lightstep

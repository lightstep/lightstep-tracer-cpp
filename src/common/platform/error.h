#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>

namespace lightstep {
#ifdef _WIN32
using ErrorCode = DWORD;
#else
using ErrorCode = int;
#endif

/**
 * Many windows functions don't use errno. This function provides a platform
 * indpendent way of getting the last error code.
 * @return the last error code set from a system call
 */
ErrorCode GetLastErrorCode() noexcept;

/**
 * Convert an error code to a human readable message.
 * @param error_code the error code to convert
 * @return the message for the given error code
 */
std::string GetErrorCodeMessage(ErrorCode error_code);

/**
 * Determine if an error code represents an "operation would block" error.
 * @param error_code the error code to check
 * @return true if the given error code is for blocking
 */
bool IsBlockingErrorCode(ErrorCode error_code) noexcept;

/**
 * Determine if an error code represents an "operation in progress" error.
 * @param error_code the error code to check
 * @return true if the given error code is for an in progress operation
 */
bool IsInProgressErrorCode(ErrorCode error_code) noexcept;
}  // namespace lightstep

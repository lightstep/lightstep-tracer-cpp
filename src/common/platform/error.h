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

ErrorCode GetLastErrorCode() noexcept;

std::string GetErrorCodeMessage(ErrorCode error_code);

bool IsBlockingErrorCode(ErrorCode error_code) noexcept;

bool IsInProgressErrorCode(ErrorCode error_code) noexcept;
} // namespace lightstep

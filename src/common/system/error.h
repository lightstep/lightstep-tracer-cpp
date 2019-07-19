#pragma once

#include <string>

namespace lightstep {
#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
using ErrorCode = DWORD;
#else
using ErrorCode = int;
#endif

ErrorCode GetLastError() noexcept;

std::string GetErrorCodeMessage(ErrorCode error_code);

bool IsBlockingErrorCode(ErrorCode error_code) noexcept;
} // namespace lightstep

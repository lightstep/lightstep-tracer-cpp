#pragma once

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
} // namespace lightstep

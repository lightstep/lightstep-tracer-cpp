#pragma once

#include "common/hex_conversion.h"

namespace lightstep {
const size_t ChunkedHttpMaxHeaderSize = Num32BitHexDigits + 2;

size_t WriteHttpChunkHeader(char* data, size_t size,
                            uint32_t chunk_size) noexcept;
}  // namespace lightstep

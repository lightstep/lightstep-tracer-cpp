#pragma once

#include "common/hex_conversion.h"

#include <opentracing/string_view.h>

namespace lightstep {
const size_t ChunkedHttpMaxHeaderSize = Num32BitHexDigits + 2;

const opentracing::string_view ChunkedHttpFooter = "\r\n";

/**
 * Write the header part of framing for an http/1.1 chunk.
 * @param data start of the buffer to write to.
 * @param size the size of the buffer
 * @param chunk_size the size of the chunk.
 *
 * Note: Data is written from the end of the provided buffer.
 */
size_t WriteHttpChunkHeader(char* data, size_t size,
                            uint32_t chunk_size) noexcept;
}  // namespace lightstep

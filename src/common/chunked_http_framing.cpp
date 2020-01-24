#include "common/chunked_http_framing.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// WriteHttpChunkHeader
//--------------------------------------------------------------------------------------------------
size_t WriteHttpChunkHeader(char* data, size_t size,
                            uint32_t chunk_size) noexcept {
  assert(size >= ChunkedHttpMaxHeaderSize);
  auto serialization_start = data + (size - ChunkedHttpMaxHeaderSize);
  auto chunk_size_str =
      Uint32ToHex(static_cast<uint32_t>(chunk_size), serialization_start);
  assert(chunk_size_str.size() == Num32BitHexDigits);
  auto iter = serialization_start + chunk_size_str.size();
  *iter++ = '\r';
  *iter++ = '\n';
  return ChunkedHttpMaxHeaderSize;
}
}  // namespace lightstep

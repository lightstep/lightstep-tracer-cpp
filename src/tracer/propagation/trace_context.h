#pragma once

#include <cstdint>

#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
const size_t TraceContextLength = 55;
const uint8_t SampledFlagMask = 1;
const uint8_t TraceContextVersion = 0;

struct TraceContext {
  uint64_t trace_id_high{0};
  uint64_t trace_id_low{0};
  uint64_t parent_id{0};
  uint8_t trace_flags{0};
  uint8_t version{TraceContextVersion};
};

opentracing::expected<void> ParseTraceContext(
    opentracing::string_view s, TraceContext& trace_context) noexcept;

void SerializeTraceContext(const TraceContext& trace_context, char* s) noexcept;

template <uint8_t Mask>
inline bool IsTraceFlagSet(uint8_t flags) noexcept {
  return static_cast<bool>(flags & Mask);
}

template <uint8_t Mask>
inline uint8_t SetTraceFlag(uint8_t flags, bool value) noexcept {
  if (value) {
    return flags | Mask;
  }
  return flags & ~Mask;
}
}  // namespace lightstep

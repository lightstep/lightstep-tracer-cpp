#pragma once

#include <cstdint>

#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
struct TraceContext {
  uint64_t trace_id_high;
  uint64_t trace_id_low;
  uint64_t parent_id;
  uint8_t trace_flags;
  uint8_t version;
};

opentracing::expected<void> ParseTraceContext(
    opentracing::string_view s, TraceContext& trace_context) noexcept;
}  // namespace lightstep

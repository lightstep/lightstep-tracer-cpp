#pragma once

#include <cstdint>

#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
struct TraceContext {
  uint64_t trace_id_high;
  uint64_t trace_id_low;
  uint64_t span_id;
  uint8_t flags;
  uint8_t version;
};

opentracing::expected<TraceContext> ParseTraceContext(opentracing::string_view s) noexcept;
} // namespace lightstep

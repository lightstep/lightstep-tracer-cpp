#pragma once

#include <cstdint>

namespace lightstep {
struct TraceContext {
  uint8_t version;
  uint64_t trace_id_high;
  uint64_t trace_id_low;
  uint64_t span_id;
  uint8_t flags;
};
} // namespace lightstep

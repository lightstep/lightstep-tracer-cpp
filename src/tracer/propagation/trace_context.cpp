#include "tracer/propagation/trace_context.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ParseTraceContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<TraceContext> ParseTraceContext(opentracing::string_view s) noexcept {
  (void)s;
  return {};
}
} // namespace lightstep

#include "tracer/propagation/trace_context.h"

#include "common/hex_conversion.h"

const size_t TraceContextMinLength = 55;

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ParseTraceContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> ParseTraceContext(
    opentracing::string_view s, TraceContext& trace_context) noexcept {
  if (s.size() < TraceContextMinLength) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }
  size_t offset = 0;

  // version
  auto version_maybe = NormalizedHexToUint8(
      opentracing::string_view{s.data() + offset, Num8BitHexDigits});
  if (!version_maybe) {
    return opentracing::make_unexpected(version_maybe.error());
  }
  trace_context.version = *version_maybe;
  offset += Num8BitHexDigits;
  if (s[offset] != '-') {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }
  ++offset;

  // trace-id
  auto error_maybe = NormalizedHexToUint128(
      opentracing::string_view{s.data() + offset, Num128BitHexDigits},
      trace_context.trace_id_high, trace_context.trace_id_low);
  if (!error_maybe) {
    return error_maybe;
  }
  offset += Num128BitHexDigits;
  if (s[offset] != '-') {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }
  ++offset;

  // parent-id
  auto parent_id_maybe = NormalizedHexToUint64(
      opentracing::string_view{s.data() + offset, Num64BitHexDigits});
  if (!parent_id_maybe) {
    return opentracing::make_unexpected(parent_id_maybe.error());
  }
  trace_context.parent_id = *parent_id_maybe;
  offset += Num64BitHexDigits;
  if (s[offset] != '-') {
    return opentracing::make_unexpected(parent_id_maybe.error());
  }
  ++offset;

  // trace-flags
  auto trace_flags_maybe = NormalizedHexToUint8(
      opentracing::string_view{s.data() + offset, Num8BitHexDigits});
  if (!trace_flags_maybe) {
    return opentracing::make_unexpected(trace_flags_maybe.error());
  }
  trace_context.trace_flags = *trace_flags_maybe;

  return {};
}
}  // namespace lightstep

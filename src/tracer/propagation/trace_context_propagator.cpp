#include "tracer/propagation/trace_context_propagator.h"

#include <array>

const opentracing::string_view TraceParentHeaderKey = "traceparent";
const opentracing::string_view TraceStateHeaderKey = "tracestate";

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// InjectSpanContextImpl
//--------------------------------------------------------------------------------------------------
static opentracing::expected<void> InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state) {
  std::array<char, TraceContextLength> buffer;
  SerializeTraceContext(trace_context, buffer.data());
  auto result =
      carrier.Set(TraceParentHeaderKey,
                  opentracing::string_view{buffer.data(), buffer.size()});
  if (!result) {
    return result;
  }
  if (trace_state.empty()) {
    return {};
  }
  return carrier.Set(TraceStateHeaderKey, trace_state);
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class KeyCompare>
static opentracing::expected<bool> ExtractSpanContextImpl(
    const opentracing::TextMapReader& carrier, TraceContext& trace_context,
    std::string& trace_state, const KeyCompare& key_compare) {
  bool parent_header_found = false;
  auto result =
      carrier.ForeachKey([&](opentracing::string_view key,
                             opentracing::string_view
                                 value) noexcept->opentracing::expected<void> {
        if (key_compare(key, TraceParentHeaderKey)) {
          auto was_successful = ParseTraceContext(value, trace_context);
          if (!was_successful) {
            return opentracing::make_unexpected(was_successful.error());
          }
          parent_header_found = true;
        } else if (key_compare(key, TraceStateHeaderKey)) {
          trace_state.assign(value.data(), value.size());
        }
        return {};
      });
  if (!result) {
    return opentracing::make_unexpected(result.error());
  }
  return parent_header_found;
}

//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> TraceContextPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageProtobufMap& /*baggage*/) const {
  return InjectSpanContextImpl(carrier, trace_context, trace_state);
}

opentracing::expected<void> TraceContextPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageFlatMap& /*baggage*/) const {
  return InjectSpanContextImpl(carrier, trace_context, trace_state);
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<bool> TraceContextPropagator::ExtractSpanContext(
    const opentracing::TextMapReader& carrier, bool case_sensitive,
    TraceContext& trace_context, std::string& trace_state,
    BaggageProtobufMap& /*baggage*/) const {
  auto iequals =
      [](opentracing::string_view lhs, opentracing::string_view rhs) noexcept {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  if (case_sensitive) {
    return ExtractSpanContextImpl(carrier, trace_context, trace_state,
                                  std::equal_to<opentracing::string_view>{});
  }
  return ExtractSpanContextImpl(carrier, trace_context, trace_state, iequals);
}
}  // namespace lightstep

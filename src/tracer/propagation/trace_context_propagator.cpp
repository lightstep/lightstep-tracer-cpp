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
  auto result = carrier.Set(
      TraceParentHeaderKey,
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
    uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
    bool& sampled, BaggageProtobufMap& baggage) const {
  (void)carrier;
  (void)case_sensitive;
  (void)trace_id_high;
  (void)trace_id_low;
  (void)span_id;
  (void)sampled;
  (void)baggage;
  return true;
}
}  // namespace lightstep

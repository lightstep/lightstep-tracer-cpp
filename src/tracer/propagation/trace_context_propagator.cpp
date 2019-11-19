#include "tracer/propagation/trace_context_propagator.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> TraceContextPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageProtobufMap& baggage) const {
  (void)carrier;
  (void)trace_id_high;
  (void)trace_id_low;
  (void)span_id;
  (void)sampled;
  (void)baggage;
  return {};
}

opentracing::expected<void> TraceContextPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageFlatMap& baggage) const {
  (void)carrier;
  (void)trace_id_high;
  (void)trace_id_low;
  (void)span_id;
  (void)sampled;
  (void)baggage;
  return {};
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

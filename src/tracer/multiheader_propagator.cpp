#include "tracer/multiheader_propagator.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MultiheaderPropagator
//--------------------------------------------------------------------------------------------------
MultiheaderPropagator::MultiheaderPropagator(
    opentracing::string_view trace_id_key, opentracing::string_view span_id_key,
    opentracing::string_view sampled_key,
    opentracing::string_view baggage_prefix) noexcept
    : trace_id_key_{trace_id_key},
      span_id_key_{span_id_key},
      sampled_key_{sampled_key},
      baggage_prefix_{baggage_prefix} {}

//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> MultiheaderPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high, uint64_t trace_id_low,
    uint64_t span_id, bool sampled, const BaggageProtobufMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, trace_id_high, trace_id_low,
                                     span_id, sampled, baggage);
}

opentracing::expected<void> MultiheaderPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageFlatMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, trace_id_high, trace_id_low,
                                     span_id, sampled, baggage);
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<bool> MultiheaderPropagator::ExtractSpanContext(
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
  return {};
}

//--------------------------------------------------------------------------------------------------
// InjectSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class BaggageMap>
opentracing::expected<void> MultiheaderPropagator::InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageMap& baggage) const {
  (void)carrier;
  (void)trace_id_high;
  (void)trace_id_low;
  (void)span_id;
  (void)sampled;
  (void)baggage;
  return {};
#if 0
  std::array<char, Num64BitHexDigits> data;
  auto result = carrier.Set(propagator.trace_id_key(),
                            Uint64ToHex(trace_id, data.data()));
  if (!result) {
    return result;
  }
  result =
      carrier.Set(propagator.span_id_key(), Uint64ToHex(span_id, data.data()));
  if (!result) {
    return result;
  }
  if (sampled) {
    result = carrier.Set(propagator.sampled_key(), TrueStr);
  } else {
    result = carrier.Set(propagator.sampled_key(), FalseStr);
  }
  if (!result) {
    return result;
  }
  if (propagator.supports_baggage() && !baggage.empty()) {
    return InjectSpanContextBaggage(propagator.baggage_prefix(), carrier,
                                    baggage);
  }
  return {};
#endif
}
}  // namespace lightstep

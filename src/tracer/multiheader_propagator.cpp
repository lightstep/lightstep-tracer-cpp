#include "tracer/multiheader_propagator.h"

#include "common/hex_conversion.h"
#include "tracer/propagation.h"

const opentracing::string_view TrueStr = "true";
const opentracing::string_view FalseStr = "false";
const opentracing::string_view OneStr = "1";
const opentracing::string_view ZeroStr = "0";

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MultiheaderPropagator
//--------------------------------------------------------------------------------------------------
MultiheaderPropagator::MultiheaderPropagator(
    opentracing::string_view trace_id_key, opentracing::string_view span_id_key,
    opentracing::string_view sampled_key,
    opentracing::string_view baggage_prefix, bool supports_128bit) noexcept
    : trace_id_key_{trace_id_key},
      span_id_key_{span_id_key},
      sampled_key_{sampled_key},
      baggage_prefix_{baggage_prefix},
      supports_128bit_{supports_128bit} {}

//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> MultiheaderPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageProtobufMap& baggage) const {
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
  HexSerializer hex_serializer;
  opentracing::expected<void> result;
  if (supports_128bit_) {
    result = carrier.Set(trace_id_key_, hex_serializer.Uint128ToHex(
                                            trace_id_high, trace_id_low));
  } else {
    result =
        carrier.Set(trace_id_key_, hex_serializer.Uint64ToHex(trace_id_low));
  }
  if (!result) {
    return result;
  }
  result = carrier.Set(span_id_key_, hex_serializer.Uint64ToHex(span_id));
  if (!result) {
    return result;
  }
  if (sampled) {
    result = carrier.Set(sampled_key_, OneStr);
  } else {
    result = carrier.Set(sampled_key_, ZeroStr);
  }
  if (!result) {
    return result;
  }
  if (!baggage.empty()) {
    return InjectSpanContextBaggage(baggage_prefix_, carrier, baggage);
  }
  return {};
}
}  // namespace lightstep

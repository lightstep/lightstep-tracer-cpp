#include "tracer/baggage_propagator.h"

#include "tracer/propagation.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
BaggagePropagator::BaggagePropagator(
    opentracing::string_view baggage_prefix) noexcept
    : baggage_prefix_{baggage_prefix} {}

//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> BaggagePropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t /*trace_id_high*/,
    uint64_t /*trace_id_low*/, uint64_t /*span_id*/, bool /*sampled*/,
    const BaggageProtobufMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, baggage);
}

opentracing::expected<void> BaggagePropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t /*trace_id_high*/,
    uint64_t /*trace_id_low*/, uint64_t /*span_id*/, bool /*sampled*/,
    const BaggageFlatMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, baggage);
}

//--------------------------------------------------------------------------------------------------
// InjectSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class BaggageMap>
opentracing::expected<void> BaggagePropagator::InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier,
    const BaggageMap& baggage) const {
  if (!baggage.empty()) {
    return InjectSpanContextBaggage(baggage_prefix_, carrier, baggage);
  }
  return {};
}
}  // namespace lightstep

#include "tracer/propagation/baggage_propagator.h"

#include <string>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// InjectSpanContextBaggage
//--------------------------------------------------------------------------------------------------
template <class BaggageMap>
static opentracing::expected<void> InjectSpanContextBaggage(
    opentracing::string_view baggage_prefix,
    const opentracing::TextMapWriter& carrier, const BaggageMap& baggage) {
  std::string baggage_key;
  try {
    baggage_key = baggage_prefix;
  } catch (const std::bad_alloc&) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::not_enough_memory));
  }
  for (const auto& baggage_item : baggage) {
    try {
      baggage_key.replace(std::begin(baggage_key) + baggage_prefix.size(),
                          std::end(baggage_key), baggage_item.first);
    } catch (const std::bad_alloc&) {
      return opentracing::make_unexpected(
          std::make_error_code(std::errc::not_enough_memory));
    }
    auto result = carrier.Set(baggage_key, baggage_item.second);
    if (!result) {
      return result;
    }
  }
  return {};
}

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
    const opentracing::TextMapWriter& carrier,
    const TraceContext& /*trace_context*/,
    opentracing::string_view /*trace_state*/,
    const BaggageProtobufMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, baggage);
}

opentracing::expected<void> BaggagePropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& /*trace_context*/,
    opentracing::string_view /*trace_state*/,
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

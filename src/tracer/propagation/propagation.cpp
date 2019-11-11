#include "tracer/propagation/propagation.h"

#include <lightstep/base64/base64.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <ios>
#include <sstream>

#include "common/in_memory_stream.h"

namespace lightstep {
//------------------------------------------------------------------------------
// InjectSpanContext
//------------------------------------------------------------------------------
template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageMap& baggage) {
  for (auto& propagator : propagation_options.inject_propagators) {
    auto was_successful = propagator->InjectSpanContext(
        carrier, trace_id_high, trace_id_low, span_id, sampled, baggage);
    if (!was_successful) {
      return was_successful;
    }
  }
  return {};
}

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageProtobufMap& baggage);

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageFlatMap& baggage);

//------------------------------------------------------------------------------
// ExtractSpanContext
//------------------------------------------------------------------------------
static opentracing::expected<bool> ExtractSpanContextImpl(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, bool case_sensitive,
    uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
    bool& sampled, BaggageProtobufMap& baggage) {
  for (auto& propagator : propagation_options.extract_propagators) {
    baggage.clear();
    auto result =
        propagator->ExtractSpanContext(carrier, case_sensitive, trace_id_high,
                                       trace_id_low, span_id, sampled, baggage);
    if (!result) {
      // One of the injected span contexts is corrupt, return immediately
      // without trying the other extractors.
      return result;
    }
    if (*result) {
      // An extractor succeeded, return without trying other extractors
      return result;
    }
  }
  return false;
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage) {
  return ExtractSpanContextImpl(propagation_options, carrier, true,
                                trace_id_high, trace_id_low, span_id, sampled,
                                baggage);
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage) {
  return ExtractSpanContextImpl(propagation_options, carrier, false,
                                trace_id_high, trace_id_low, span_id, sampled,
                                baggage);
}
}  // namespace lightstep

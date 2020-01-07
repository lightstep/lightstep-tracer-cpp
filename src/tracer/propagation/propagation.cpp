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
//--------------------------------------------------------------------------------------------------
// ExtractSpanContextImpl
//--------------------------------------------------------------------------------------------------
static opentracing::expected<bool> ExtractSpanContextImpl(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, bool case_sensitive,
    TraceContext& trace_context, std::string& trace_state,
    BaggageProtobufMap& baggage) {
  for (auto& propagator : propagation_options.extract_propagators) {
    baggage.clear();
    auto result = propagator->ExtractSpanContext(
        carrier, case_sensitive, trace_context, trace_state, baggage);
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

//------------------------------------------------------------------------------
// InjectSpanContext
//------------------------------------------------------------------------------
template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageMap& baggage) {
  for (auto& propagator : propagation_options.inject_propagators) {
    auto was_successful = propagator->InjectSpanContext(carrier, trace_context,
                                                        trace_state, baggage);
    if (!was_successful) {
      return was_successful;
    }
  }
  return {};
}

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageProtobufMap& baggage);

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageFlatMap& baggage);

//------------------------------------------------------------------------------
// ExtractSpanContext
//------------------------------------------------------------------------------
opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, TraceContext& trace_context,
    std::string& trace_state, BaggageProtobufMap& baggage) {
  return ExtractSpanContextImpl(propagation_options, carrier, true,
                                trace_context, trace_state, baggage);
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, TraceContext& trace_context,
    std::string& trace_state, BaggageProtobufMap& baggage) {
  return ExtractSpanContextImpl(propagation_options, carrier, false,
                                trace_context, trace_state, baggage);
}
}  // namespace lightstep

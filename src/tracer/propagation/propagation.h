#pragma once

#include <lightstep/tracer.h>

#include "common/utility.h"
#include "lightstep-tracer-common/lightstep_carrier.pb.h"
#include "tracer/baggage_flat_map.h"
#include "tracer/propagation/binary_propagation.h"
#include "tracer/propagation/propagation_options.h"

#include <google/protobuf/map.h>
#include <opentracing/propagation.h>

namespace lightstep {
template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& /*propagation_options*/, std::ostream& carrier,
    uint64_t /*trace_id_high*/, uint64_t trace_id, uint64_t span_id,
    bool sampled, const BaggageMap& baggage) {
  return InjectSpanContext(carrier, trace_id, span_id, sampled, baggage);
}

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& /*propagation_options*/, std::ostream& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageMap& baggage) {
  return InjectSpanContext(
      carrier, trace_context.trace_id_low, trace_context.parent_id,
      IsTraceFlagSet<SampledFlagMask>(trace_context.trace_flags), baggage);
}

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
    uint64_t trace_id_low, uint64_t span_id, bool sampled,
    const BaggageMap& baggage);

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view trace_state,
    const BaggageMap& baggage);

inline opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& /*propagation_options*/, std::istream& carrier,
    TraceContext& trace_context, std::string& /*trace_state*/,
    BaggageProtobufMap& baggage) {
  trace_context.trace_id_high = 0;
  bool sampled;
  auto result = ExtractSpanContext(carrier, trace_context.trace_id_low,
                                   trace_context.parent_id, sampled, baggage);
  if (!result || !*result) {
    return result;
  }
  trace_context.trace_flags = SetTraceFlag<SampledFlagMask>(0, sampled);
  return result;
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, TraceContext& trace_context,
    std::string& trace_state, BaggageProtobufMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, TraceContext& trace_context,
    std::string& trace_state, BaggageProtobufMap& baggage);
}  // namespace lightstep

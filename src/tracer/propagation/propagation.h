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
    uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
    bool& sampled, BaggageProtobufMap& baggage) {
  trace_id_high = 0;
  return ExtractSpanContext(carrier, trace_id_low, span_id, sampled, baggage);
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage);
}  // namespace lightstep

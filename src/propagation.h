#pragma once

#include <google/protobuf/map.h>
#include <opentracing/propagation.h>
#include <unordered_map>

namespace lightstep {
using BaggageMap = google::protobuf::Map<std::string, std::string>;

struct PropagationOptions {
  bool use_single_key = false;
};

opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options, std::ostream& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageMap& baggage);

opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options, std::istream& carrier,
    uint64_t& trace_id, uint64_t& span_id, bool& sampled, BaggageMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageMap& baggage);
}  // namespace lightstep

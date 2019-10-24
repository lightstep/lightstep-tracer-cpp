#pragma once

#include <lightstep/tracer.h>

#include "tracer/baggage_flat_map.h"

#include <google/protobuf/map.h>
#include <opentracing/propagation.h>

namespace lightstep {
using BaggageProtobufMap = google::protobuf::Map<std::string, std::string>;

struct PropagationOptions {
  PropagationMode propagation_mode;
  bool use_single_key = false;
};

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options, std::ostream& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageMap& baggage);

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options, std::istream& carrier,
    uint64_t& trace_id, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage);

template <class BaggageMap>
opentracing::expected<void> InjectSpanContextBaggage(
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
}  // namespace lightstep

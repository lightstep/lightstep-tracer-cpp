#pragma once

#include <lightstep/tracer.h>

#include "common/utility.h"
#include "lightstep-tracer-common/lightstep_carrier.pb.h"
#include "tracer/baggage_flat_map.h"
#include "tracer/propagation_options.h"

#include <google/protobuf/map.h>
#include <opentracing/propagation.h>

namespace lightstep {
using BaggageProtobufMap = google::protobuf::Map<std::string, std::string>;

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

template <class KeyCompare>
opentracing::expected<opentracing::string_view> LookupKey(
    const opentracing::TextMapReader& carrier, opentracing::string_view key,
    KeyCompare key_compare) {
  // First try carrier.LookupKey since that can potentially be the fastest
  // approach.
  auto result = carrier.LookupKey(key);
  if (result || !AreErrorsEqual(result.error(),
                                opentracing::lookup_key_not_supported_error)) {
    return result;
  }

  // Fall back to iterating through all of the keys.
  result = opentracing::make_unexpected(opentracing::key_not_found_error);
  auto was_successful = carrier.ForeachKey(
      [&](opentracing::string_view carrier_key,
          opentracing::string_view value) -> opentracing::expected<void> {
        if (!key_compare(carrier_key, key)) {
          return {};
        }
        result = value;

        // Found key, so bail out of the loop with a success error code.
        return opentracing::make_unexpected(std::error_code{});
      });
  if (!was_successful && was_successful.error() != std::error_code{}) {
    return opentracing::make_unexpected(was_successful.error());
  }
  return result;
}
}  // namespace lightstep

#include "tracer/propagation.h"
#include <lightstep/base64/base64.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <ios>
#include <sstream>

#include "common/hex_conversion.h"
#include "common/in_memory_stream.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ToLower
//--------------------------------------------------------------------------------------------------
static BaggageProtobufMap ToLower(const BaggageProtobufMap& baggage) {
  BaggageProtobufMap result;
  for (auto& baggage_item : baggage) {
    result.insert(BaggageProtobufMap::value_type(ToLower(baggage_item.first),
                                                 baggage_item.second));
  }
  return result;
}

//------------------------------------------------------------------------------
// InjectSpanContext
//------------------------------------------------------------------------------
template <class BaggageMap>
static opentracing::expected<void> InjectSpanContext(
    BinaryCarrier& carrier, uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageMap& baggage) noexcept try {
  carrier.Clear();
  auto basic = carrier.mutable_basic_ctx();
  basic->set_trace_id(trace_id);
  basic->set_span_id(span_id);
  basic->set_sampled(sampled);
  auto mutable_baggage = basic->mutable_baggage_items();
  for (auto& baggage_item : baggage) {
    (*mutable_baggage)[baggage_item.first] = baggage_item.second;
  }
  return {};
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& /*propagation_options*/, std::ostream& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageMap& baggage) {
  BinaryCarrier binary_carrier;
  auto result =
      InjectSpanContext(binary_carrier, trace_id, span_id, sampled, baggage);
  if (!result) {
    return result;
  }
  if (!binary_carrier.SerializeToOstream(&carrier)) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::io_error));
  }

  // Flush so that when we call carrier.good(), we'll get an accurate view of
  // the error state.
  carrier.flush();
  if (!carrier.good()) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::io_error));
  }

  return {};
}

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& /*propagation_options*/, std::ostream& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageProtobufMap& baggage);

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& /*propagation_options*/, std::ostream& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageFlatMap& baggage);

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
static opentracing::expected<bool> ExtractSpanContext(
    const BinaryCarrier& carrier, uint64_t& trace_id, uint64_t& span_id,
    bool& sampled, BaggageProtobufMap& baggage) noexcept try {
  auto& basic = carrier.basic_ctx();
  trace_id = basic.trace_id();
  span_id = basic.span_id();
  sampled = basic.sampled();
  baggage = ToLower(basic.baggage_items());
  return true;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& /*propagation_options*/, std::istream& carrier,
    uint64_t& trace_id, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage) try {
  // istream::peek returns EOF if it's in an error state, so check for an error
  // state first before checking for an empty stream.
  if (!carrier.good()) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::io_error));
  }

  // Check for the case when no span is encoded.
  if (carrier.peek() == EOF) {
    return false;
  }

  BinaryCarrier binary_carrier;
  if (!binary_carrier.ParseFromIstream(&carrier)) {
    return opentracing::make_unexpected(
        opentracing::span_context_corrupted_error);
  }
  return ExtractSpanContext(binary_carrier, trace_id, span_id, sampled,
                            baggage);
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

static opentracing::expected<bool> ExtractSpanContextImpl(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, bool case_sensitive,
    uint64_t trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
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

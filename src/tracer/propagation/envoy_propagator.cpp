#include "tracer/propagation/envoy_propagator.h"

#include <lightstep/base64/base64.h>

#include "common/in_memory_stream.h"
#include "tracer/propagation/binary_propagation.h"
#include "tracer/propagation/utility.h"

const opentracing::string_view PropagationSingleKey = "x-ot-span-context";

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> EnvoyPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageProtobufMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, trace_context, baggage);
}

opentracing::expected<void> EnvoyPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageFlatMap& baggage) const {
  return this->InjectSpanContextImpl(carrier, trace_context, baggage);
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<bool> EnvoyPropagator::ExtractSpanContext(
    const opentracing::TextMapReader& carrier, bool case_sensitive,
    TraceContext& trace_context, std::string& /*trace_state*/,
    BaggageProtobufMap& baggage) const {
  auto iequals =
      [](opentracing::string_view lhs, opentracing::string_view rhs) noexcept {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  bool sampled;
  opentracing::expected<bool> result;
  if (case_sensitive) {
    result = ExtractSpanContextImpl(carrier, trace_context.trace_id_high,
                                    trace_context.trace_id_low,
                                    trace_context.parent_id, sampled, baggage,
                                    std::equal_to<opentracing::string_view>{});
  } else {
    result = result = ExtractSpanContextImpl(
        carrier, trace_context.trace_id_high, trace_context.trace_id_low,
        trace_context.parent_id, sampled, baggage, iequals);
  }
  if (!result || !*result) {
    return result;
  }
  trace_context.trace_flags = SetTraceFlag<SampledFlagMask>(0, sampled);
  return result;
}

//--------------------------------------------------------------------------------------------------
// InjectSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class BaggageMap>
opentracing::expected<void> EnvoyPropagator::InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, const BaggageMap& baggage) const {
  std::ostringstream ostream;
  auto result = lightstep::InjectSpanContext(
      ostream, trace_context.trace_id_low, trace_context.parent_id,
      IsTraceFlagSet<SampledFlagMask>(trace_context.trace_flags), baggage);
  if (!result) {
    return result;
  }
  std::string context_value;
  try {
    auto binary_encoding = ostream.str();
    context_value =
        Base64::encode(binary_encoding.data(), binary_encoding.size());
  } catch (const std::bad_alloc&) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::not_enough_memory));
  }

  result = carrier.Set(PropagationSingleKey, context_value);
  if (!result) {
    return result;
  }
  return {};
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class KeyCompare>
opentracing::expected<bool> EnvoyPropagator::ExtractSpanContextImpl(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage, const KeyCompare& key_compare) const {
  trace_id_high = 0;
  auto value_maybe = LookupKey(carrier, PropagationSingleKey, key_compare);
  if (!value_maybe) {
    if (AreErrorsEqual(value_maybe.error(), opentracing::key_not_found_error)) {
      return false;
    }
    return opentracing::make_unexpected(value_maybe.error());
  }
  auto value = *value_maybe;
  std::string base64_decoding;
  try {
    base64_decoding = Base64::decode(value.data(), value.size());
  } catch (const std::bad_alloc&) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::not_enough_memory));
  }
  if (base64_decoding.empty()) {
    return opentracing::make_unexpected(
        opentracing::span_context_corrupted_error);
  }
  in_memory_stream istream{base64_decoding.data(), base64_decoding.size()};
  return lightstep::ExtractSpanContext(istream, trace_id_low, span_id, sampled,
                                       baggage);
}
}  // namespace lightstep

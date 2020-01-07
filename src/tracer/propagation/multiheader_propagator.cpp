#include "tracer/propagation/multiheader_propagator.h"

#include "common/hex_conversion.h"
#include "common/utility.h"

const int FieldCount = 2;
const opentracing::string_view FalseStr = "false";
const opentracing::string_view OneStr = "1";
const opentracing::string_view ZeroStr = "0";

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MultiheaderPropagator
//--------------------------------------------------------------------------------------------------
MultiheaderPropagator::MultiheaderPropagator(
    opentracing::string_view trace_id_key, opentracing::string_view span_id_key,
    opentracing::string_view sampled_key,
    opentracing::string_view baggage_prefix, bool supports_128bit) noexcept
    : trace_id_key_{trace_id_key},
      span_id_key_{span_id_key},
      sampled_key_{sampled_key},
      baggage_prefix_{baggage_prefix},
      supports_128bit_{supports_128bit} {}

//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> MultiheaderPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageProtobufMap& /*baggage*/) const {
  return this->InjectSpanContextImpl(carrier, trace_context);
}

opentracing::expected<void> MultiheaderPropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageFlatMap& /*baggage*/) const {
  return this->InjectSpanContextImpl(carrier, trace_context);
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<bool> MultiheaderPropagator::ExtractSpanContext(
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
opentracing::expected<void> MultiheaderPropagator::InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context) const {
  HexSerializer hex_serializer;
  opentracing::expected<void> result;
  if (supports_128bit_) {
    result = carrier.Set(
        trace_id_key_, hex_serializer.Uint128ToHex(trace_context.trace_id_high,
                                                   trace_context.trace_id_low));
  } else {
    result = carrier.Set(
        trace_id_key_, hex_serializer.Uint64ToHex(trace_context.trace_id_low));
  }
  if (!result) {
    return result;
  }
  result = carrier.Set(span_id_key_,
                       hex_serializer.Uint64ToHex(trace_context.parent_id));
  if (!result) {
    return result;
  }
  if (IsTraceFlagSet<SampledFlagMask>(trace_context.trace_flags)) {
    result = carrier.Set(sampled_key_, OneStr);
  } else {
    result = carrier.Set(sampled_key_, ZeroStr);
  }
  if (!result) {
    return result;
  }
  return {};
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class KeyCompare>
opentracing::expected<bool> MultiheaderPropagator::ExtractSpanContextImpl(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
    uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage, const KeyCompare& key_compare) const {
  sampled = true;
  int count = 0;
  auto result =
      carrier.ForeachKey([&](opentracing::string_view key,
                             opentracing::string_view
                                 value) noexcept->opentracing::expected<void> {
        try {
          if (key_compare(key, trace_id_key_)) {
            if (!HexToUint128(value, trace_id_high, trace_id_low)) {
              return opentracing::make_unexpected(
                  opentracing::span_context_corrupted_error);
            }
            ++count;
          } else if (key_compare(key, span_id_key_)) {
            auto span_id_maybe = HexToUint64(value);
            if (!span_id_maybe) {
              return opentracing::make_unexpected(
                  opentracing::span_context_corrupted_error);
            }
            span_id = *span_id_maybe;
            ++count;
          } else if (key_compare(key, sampled_key_)) {
            sampled = !(value == FalseStr || value == ZeroStr);
          } else if (key.length() > baggage_prefix_.size() &&
                     key_compare(
                         opentracing::string_view{key.data(),
                                                  baggage_prefix_.size()},
                         baggage_prefix_)) {
            baggage.insert(BaggageProtobufMap::value_type(
                ToLower(opentracing::string_view{
                    key.data() + baggage_prefix_.size(),
                    key.size() - baggage_prefix_.size()}),
                value));
          }
          return {};
        } catch (const std::bad_alloc&) {
          return opentracing::make_unexpected(
              std::make_error_code(std::errc::not_enough_memory));
        }
      });
  if (!result) {
    return opentracing::make_unexpected(result.error());
  }
  if (count == 0) {
    return false;
  }
  if (count > 0 && count != FieldCount) {
    return opentracing::make_unexpected(
        opentracing::span_context_corrupted_error);
  }
  return true;
}
}  // namespace lightstep

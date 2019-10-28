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

#include "b3_propagator.h"
#include "common/hex_conversion.h"
#include "common/in_memory_stream.h"
#include "lightstep_propagator.h"

namespace lightstep {
const int FieldCount = 3;

const opentracing::string_view PropagationSingleKey = "x-ot-span-context";
const opentracing::string_view TrueStr = "true";
const opentracing::string_view FalseStr = "false";
const opentracing::string_view ZeroStr = "0";

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
// InjectSpanContextMultiKey
//------------------------------------------------------------------------------
template <class Propagator, class BaggageMap>
static opentracing::expected<void> InjectSpanContextMultiKey(
    const Propagator& propagator, const opentracing::TextMapWriter& carrier,
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const BaggageMap& baggage) {
  std::array<char, Num64BitHexDigits> data;
  auto result = carrier.Set(propagator.trace_id_key(),
                            Uint64ToHex(trace_id, data.data()));
  if (!result) {
    return result;
  }
  result =
      carrier.Set(propagator.span_id_key(), Uint64ToHex(span_id, data.data()));
  if (!result) {
    return result;
  }
  if (sampled) {
    result = carrier.Set(propagator.sampled_key(), TrueStr);
  } else {
    result = carrier.Set(propagator.sampled_key(), FalseStr);
  }
  if (!result) {
    return result;
  }
  if (propagator.supports_baggage() && !baggage.empty()) {
    return InjectSpanContextBaggage(propagator.baggage_prefix(), carrier,
                                    baggage);
  }
  return {};
}

//------------------------------------------------------------------------------
// InjectSpanContextSingleKey
//------------------------------------------------------------------------------
template <class BaggageMap>
static opentracing::expected<void> InjectSpanContextSingleKey(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageMap& baggage) {
  std::ostringstream ostream;
  auto result = InjectSpanContext(PropagationOptions{}, ostream, trace_id,
                                  span_id, sampled, baggage);
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
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageMap& baggage) {
  if (propagation_options.use_single_key) {
    return InjectSpanContextSingleKey(carrier, trace_id, span_id, sampled,
                                      baggage);
  }
  if (propagation_options.propagation_mode == PropagationMode::b3) {
    return InjectSpanContextMultiKey(B3Propagator{}, carrier, trace_id, span_id,
                                     sampled, baggage);
  }
  return InjectSpanContextMultiKey(LightStepPropagator{}, carrier, trace_id,
                                   span_id, sampled, baggage);
}

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
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageProtobufMap& baggage);

template opentracing::expected<void> InjectSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id, bool sampled, const BaggageFlatMap& baggage);

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
// ExtractSpanContextMultiKey
//------------------------------------------------------------------------------
template <class Propagator, class KeyCompare>
static opentracing::expected<bool> ExtractSpanContextMultiKey(
    const Propagator& propagator, const opentracing::TextMapReader& carrier,
    uint64_t& trace_id, uint64_t& span_id, bool& sampled,
    BaggageProtobufMap& baggage, KeyCompare key_compare) {
  int count = 0;
  auto baggage_prefix = propagator.baggage_prefix();
  auto result = carrier.ForeachKey(
      [&](opentracing::string_view key,
          opentracing::string_view value) -> opentracing::expected<void> {
        try {
          if (key_compare(key, propagator.trace_id_key())) {
            auto trace_id_maybe = HexToUint64(value);
            if (!trace_id_maybe) {
              return opentracing::make_unexpected(
                  opentracing::span_context_corrupted_error);
            }
            trace_id = *trace_id_maybe;
            count++;
          } else if (key_compare(key, propagator.span_id_key())) {
            auto span_id_maybe = HexToUint64(value);
            if (!span_id_maybe) {
              return opentracing::make_unexpected(
                  opentracing::span_context_corrupted_error);
            }
            span_id = *span_id_maybe;
            count++;
          } else if (key_compare(key, propagator.sampled_key())) {
            sampled = !(value == FalseStr || value == ZeroStr);
            count++;
          } else if (propagator.supports_baggage() &&
                     key.length() > baggage_prefix.size() &&
                     key_compare(
                         opentracing::string_view{key.data(),
                                                  baggage_prefix.size()},
                         baggage_prefix)) {
            baggage.insert(BaggageProtobufMap::value_type(
                ToLower(opentracing::string_view{
                    key.data() + baggage_prefix.size(),
                    key.size() - baggage_prefix.size()}),
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

//------------------------------------------------------------------------------
// ExtractSpanContextSingleKey
//------------------------------------------------------------------------------
template <class KeyCompare>
static opentracing::expected<bool> ExtractSpanContextSingleKey(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage,
    KeyCompare key_compare) {
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
  return ExtractSpanContext(PropagationOptions{}, istream, trace_id, span_id,
                            sampled, baggage);
}

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

template <class KeyCompare>
static opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage,
    KeyCompare key_compare) {
  if (propagation_options.use_single_key) {
    auto span_context_maybe = ExtractSpanContextSingleKey(
        carrier, trace_id, span_id, sampled, baggage, key_compare);

    // If no span context was found, fall back to multikey extraction so as to
    // support interoperability with other tracers.
    if (!span_context_maybe || *span_context_maybe) {
      return span_context_maybe;
    }
  }
  if (propagation_options.propagation_mode == PropagationMode::b3) {
    return ExtractSpanContextMultiKey(B3Propagator{}, carrier, trace_id,
                                      span_id, sampled, baggage, key_compare);
  }
  return ExtractSpanContextMultiKey(LightStepPropagator{}, carrier, trace_id,
                                    span_id, sampled, baggage, key_compare);
}

opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage) {
  return ExtractSpanContext(propagation_options, carrier, trace_id, span_id,
                            sampled, baggage,
                            std::equal_to<opentracing::string_view>());
}

// HTTP header field names are case insensitive, so we need to ignore case when
// comparing against the OpenTracing field names.
//
// See https://stackoverflow.com/a/5259004/4447365
opentracing::expected<bool> ExtractSpanContext(
    const PropagationOptions& propagation_options,
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, bool& sampled, BaggageProtobufMap& baggage) {
  auto iequals = [](opentracing::string_view lhs,
                    opentracing::string_view rhs) {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  return ExtractSpanContext(propagation_options, carrier, trace_id, span_id,
                            sampled, baggage, iequals);
}
}  // namespace lightstep

#include "propagation.h"
#include <lightstep_carrier.pb.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <ios>
#include <sstream>

namespace lightstep {
#define PREFIX_TRACER_STATE "ot-tracer-"
// Note: these constants are a convention of the OpenTracing basictracers.
const opentracing::string_view PrefixBaggage = "ot-baggage-";

const int FieldCount = 3;
const opentracing::string_view FieldNameTraceID = PREFIX_TRACER_STATE "traceid";
const opentracing::string_view FieldNameSpanID = PREFIX_TRACER_STATE "spanid";
const opentracing::string_view FieldNameSampled = PREFIX_TRACER_STATE "sampled";
#undef PREFIX_TRACER_STATE

//------------------------------------------------------------------------------
// Uint64ToHex
//------------------------------------------------------------------------------
static std::string Uint64ToHex(uint64_t u) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(16) << std::hex << u;
  return ss.str();
}

//------------------------------------------------------------------------------
// HexToUint64
//------------------------------------------------------------------------------
static uint64_t HexToUint64(const std::string& s) {
  std::stringstream ss(s);
  uint64_t x;
  ss >> std::setw(16) >> std::hex >> x;
  return x;
}

//------------------------------------------------------------------------------
// InjectSpanContext
//------------------------------------------------------------------------------
static opentracing::expected<void> InjectSpanContext(
    BinaryCarrier& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) noexcept try {
  carrier.Clear();
  auto basic = carrier.mutable_basic_ctx();
  basic->set_trace_id(trace_id);
  basic->set_span_id(span_id);
  basic->set_sampled(true);
  auto mutable_baggage = basic->mutable_baggage_items();
  for (auto& baggage_item : baggage) {
    (*mutable_baggage)[baggage_item.first] = baggage_item.second;
  }
  return {};
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

opentracing::expected<void> InjectSpanContext(
    std::ostream& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  BinaryCarrier binary_carrier;
  auto result = InjectSpanContext(binary_carrier, trace_id, span_id, baggage);
  if (!result) {
    return result;
  }
  if (!binary_carrier.SerializeToOstream(&carrier)) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::io_error));
  }

  // Flush so that when we call carrier.good, we'll get an accurate view of the
  // error state.
  carrier.flush();
  if (!carrier.good()) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::io_error));
  }

  return {};
}

opentracing::expected<void> InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  std::string trace_id_hex, span_id_hex, baggage_key;
  try {
    trace_id_hex = Uint64ToHex(trace_id);
    span_id_hex = Uint64ToHex(span_id);
    baggage_key = PrefixBaggage;
  } catch (const std::bad_alloc&) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::not_enough_memory));
  }
  auto result = carrier.Set(FieldNameTraceID, trace_id_hex);
  if (!result) {
    return result;
  }
  result = carrier.Set(FieldNameSpanID, span_id_hex);
  if (!result) {
    return result;
  }
  result = carrier.Set(FieldNameSampled, "true");
  if (!result) {
    return result;
  }
  for (const auto& baggage_item : baggage) {
    try {
      baggage_key.replace(std::begin(baggage_key) + PrefixBaggage.size(),
                          std::end(baggage_key), baggage_item.first);
    } catch (const std::bad_alloc&) {
      return opentracing::make_unexpected(
          std::make_error_code(std::errc::not_enough_memory));
    }
    result = carrier.Set(baggage_key, baggage_item.second);
    if (!result) {
      return result;
    }
  }
  return {};
}

//------------------------------------------------------------------------------
// ExtractSpanContext
//------------------------------------------------------------------------------
static opentracing::expected<bool> ExtractSpanContext(
    const BinaryCarrier& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) noexcept try {
  auto& basic = carrier.basic_ctx();
  trace_id = basic.trace_id();
  span_id = basic.span_id();
  for (const auto& entry : basic.baggage_items()) {
    baggage.emplace(entry.first, entry.second);
  }
  return true;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

opentracing::expected<bool> ExtractSpanContext(
    std::istream& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) try {
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
    return false;
  }
  return ExtractSpanContext(binary_carrier, trace_id, span_id, baggage);
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

template <class KeyCompare>
static opentracing::expected<bool> ExtractSpanContext(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage,
    KeyCompare key_compare) {
  int count = 0;
  auto result = carrier.ForeachKey(
      [&](opentracing::string_view key,
          opentracing::string_view value) -> opentracing::expected<void> {
        try {
          if (key_compare(key, FieldNameTraceID)) {
            trace_id = HexToUint64(value);
            count++;
          } else if (key_compare(key, FieldNameSpanID)) {
            span_id = HexToUint64(value);
            count++;
          } else if (key_compare(key, FieldNameSampled)) {
            // Ignored
            count++;
          } else if (key.length() > PrefixBaggage.size() &&
                     key_compare(opentracing::string_view{key.data(),
                                                          PrefixBaggage.size()},
                                 PrefixBaggage)) {
            baggage.emplace(std::string{std::begin(key) + PrefixBaggage.size(),
                                        std::end(key)},
                            value);
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

opentracing::expected<bool> ExtractSpanContext(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage) {
  return ExtractSpanContext(carrier, trace_id, span_id, baggage,
                            std::equal_to<opentracing::string_view>());
}

// HTTP header field names are case insensitive, so we need to ignore case when
// comparing against the OpenTracing field names.
//
// See https://stackoverflow.com/a/5259004/4447365
opentracing::expected<bool> ExtractSpanContext(
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage) {
  auto iequals = [](opentracing::string_view lhs,
                    opentracing::string_view rhs) {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  return ExtractSpanContext(carrier, trace_id, span_id, baggage, iequals);
}
}  // namespace lightstep

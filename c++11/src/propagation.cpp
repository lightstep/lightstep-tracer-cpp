#include "propagation.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <sstream>
using namespace opentracing;

namespace lightstep {
#define PREFIX_TRACER_STATE "ot-tracer-"
// Note: these constants are a convention of the OpenTracing basictracers.
const std::string PrefixBaggage = "ot-baggage-";

const int FieldCount = 3;
const std::string FieldNameTraceID = PREFIX_TRACER_STATE "traceid";
const std::string FieldNameSpanID = PREFIX_TRACER_STATE "spanid";
const std::string FieldNameSampled = PREFIX_TRACER_STATE "sampled";
#undef PREFIX_TRACER_STATE
//------------------------------------------------------------------------------
// uint64_to_hex
//------------------------------------------------------------------------------
static std::string uint64_to_hex(uint64_t u) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(16) << std::hex << u;
  return ss.str();
}

//------------------------------------------------------------------------------
// hex_to_uint64
//------------------------------------------------------------------------------
static uint64_t hex_to_uint64(const std::string& s) {
  std::stringstream ss(s);
  uint64_t x;
  ss >> std::setw(16) >> std::hex >> x;
  return x;
}

//------------------------------------------------------------------------------
// inject_span_context
//------------------------------------------------------------------------------
Expected<void> inject_span_context(
    const TextMapWriter& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  std::string trace_id_hex, span_id_hex, baggage_key;
  try {
    trace_id_hex = uint64_to_hex(trace_id);
    span_id_hex = uint64_to_hex(span_id);
    baggage_key = PrefixBaggage;
  } catch (const std::bad_alloc&) {
    return make_unexpected(make_error_code(std::errc::not_enough_memory));
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
      return make_unexpected(make_error_code(std::errc::not_enough_memory));
    }
    result = carrier.Set(baggage_key, baggage_item.second);
    if (!result) {
      return result;
    }
  }
  return {};
}

//------------------------------------------------------------------------------
// extract_span_context
//------------------------------------------------------------------------------
template <class KeyCompare>
static Expected<bool> extract_span_context(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage,
    KeyCompare key_compare) {
  int count = 0;
  auto result = carrier.ForeachKey([&](StringRef key,
                                       StringRef value) -> Expected<void> {
    if (key_compare(key, FieldNameTraceID)) {
      trace_id = hex_to_uint64(value);
      count++;
    } else if (key_compare(key, FieldNameSpanID)) {
      span_id = hex_to_uint64(value);
      count++;
    } else if (key_compare(key, FieldNameSampled)) {
      // Ignored
      count++;
    } else if (key.length() > PrefixBaggage.size() &&
               key_compare(StringRef(key.data(), PrefixBaggage.size()),
                           PrefixBaggage)) {
      try {
        baggage.emplace(
            std::string(std::begin(key) + PrefixBaggage.size(), std::end(key)),
            value);
      } catch (const std::bad_alloc&) {
        return make_unexpected(make_error_code(std::errc::not_enough_memory));
      }
    }
    return {};
  });
  if (!result) {
    return make_unexpected(result.error());
  }
  if (count == 0) {
    return false;
  }
  if (count > 0 && count != FieldCount) {
    return make_unexpected(span_context_corrupted_error);
  }
  { return true; }
}

Expected<bool> extract_span_context(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  return extract_span_context(carrier, trace_id, span_id, baggage,
                              std::equal_to<StringRef>());
}

// HTTP header field names are case insensitive, so we need to ignore case when
// comparing against the OpenTracing field names.
//
// See https://stackoverflow.com/a/5259004/4447365
Expected<bool> extract_span_context(
    const HTTPHeadersReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  auto iequals = [](StringRef lhs, StringRef rhs) {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  return extract_span_context(carrier, trace_id, span_id, baggage, iequals);
}
}  // namespace lightstep

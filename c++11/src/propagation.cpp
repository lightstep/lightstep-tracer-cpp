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
const string_view PrefixBaggage = "ot-baggage-";

const int FieldCount = 3;
const string_view FieldNameTraceID = PREFIX_TRACER_STATE "traceid";
const string_view FieldNameSpanID = PREFIX_TRACER_STATE "spanid";
const string_view FieldNameSampled = PREFIX_TRACER_STATE "sampled";
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
// WriteUint64
//------------------------------------------------------------------------------
static void WriteUint64(std::ostream& out, uint64_t u) {
  out.write(reinterpret_cast<char*>(&u), sizeof(u));
}

//------------------------------------------------------------------------------
// ReadUint64
//------------------------------------------------------------------------------
static uint64_t ReadUint64(std::istream& in) {
  uint64_t u = 0;
  in.read(reinterpret_cast<char*>(&u), sizeof(u));
  if (!in.good()) {
    return 0;
  }
  return u;
}

//------------------------------------------------------------------------------
// WriteString
//------------------------------------------------------------------------------
static void WriteString(std::ostream& out, const std::string& s) {
  WriteUint64(out, s.size());
  out.write(s.data(), s.size());
}

//------------------------------------------------------------------------------
// ReadString
//------------------------------------------------------------------------------
static void ReadString(std::istream& in, std::string& s) {
  auto length = ReadUint64(in);
  s.resize(length);
  in.read(&s.front(), length);
}

//------------------------------------------------------------------------------
// InjectSpanContext
//------------------------------------------------------------------------------
expected<void> InjectSpanContext(
    std::ostream& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  // TODO(rnburn): Do we want to fiddle with carrier.exceptions() to ensure that
  // exceptions aren't thrown?
  WriteUint64(carrier, trace_id);
  WriteUint64(carrier, span_id);
  WriteUint64(carrier, baggage.size());
  for (const auto& baggage_item : baggage) {
    WriteString(carrier, baggage_item.first);
    WriteString(carrier, baggage_item.second);
  }

  // Flush so that when we call carrier.good, we'll get an accurate view of the
  // error state.
  carrier.flush();
  if (!carrier.good()) {
    return make_unexpected(std::make_error_code(std::io_errc::stream));
  }

  return {};
}

expected<void> InjectSpanContext(
    const TextMapWriter& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  std::string trace_id_hex, span_id_hex, baggage_key;
  try {
    trace_id_hex = Uint64ToHex(trace_id);
    span_id_hex = Uint64ToHex(span_id);
    baggage_key = PrefixBaggage;
  } catch (const std::bad_alloc&) {
    return make_unexpected(std::make_error_code(std::errc::not_enough_memory));
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
      return make_unexpected(
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
expected<bool> ExtractSpanContext(
    std::istream& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) try {
  // istream::peek returns EOF if it's in an error state, so check for an error
  // state first before checking for an empty stream.
  if (!carrier.good()) {
    return make_unexpected(std::make_error_code(std::io_errc::stream));
  }

  // Check for the case when no span is encoded.
  if (carrier.peek() == EOF) {
    return false;
  }

  // TODO(rnburn): Do we want to fiddle with carrier.exceptions() to ensure that
  // exceptions aren't thrown?
  trace_id = ReadUint64(carrier);
  span_id = ReadUint64(carrier);
  auto num_baggage = ReadUint64(carrier);
  baggage.reserve(num_baggage);
  std::string key, value;
  for (int i = 0; i < num_baggage; ++i) {
    ReadString(carrier, key);
    ReadString(carrier, value);
    baggage[key] = value;

    // Check fo EOF and break out of the loop if reached in case we're reading
    // invalid data and `num_baggage` is some arbitrarily large value.
    if (carrier.eof()) {
      break;
    }
  }
  if (carrier.eof()) {
    return make_unexpected(span_context_corrupted_error);
  }
  if (!carrier.good()) {
    return make_unexpected(std::make_error_code(std::io_errc::stream));
  }
  return true;
} catch (const std::bad_alloc&) {
  return make_unexpected(std::make_error_code(std::errc::not_enough_memory));
}

template <class KeyCompare>
static expected<bool> ExtractSpanContext(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage,
    KeyCompare key_compare) {
  int count = 0;
  auto result = carrier.ForeachKey([&](string_view key,
                                       string_view value) -> expected<void> {
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
                 key_compare(string_view{key.data(), PrefixBaggage.size()},
                             PrefixBaggage)) {
        baggage.emplace(
            std::string{std::begin(key) + PrefixBaggage.size(), std::end(key)},
            value);
      }
      return {};
    } catch (const std::bad_alloc&) {
      return make_unexpected(
          std::make_error_code(std::errc::not_enough_memory));
    }
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
  return true;
}

expected<bool> ExtractSpanContext(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  return ExtractSpanContext(carrier, trace_id, span_id, baggage,
                            std::equal_to<string_view>());
}

// HTTP header field names are case insensitive, so we need to ignore case when
// comparing against the OpenTracing field names.
//
// See https://stackoverflow.com/a/5259004/4447365
expected<bool> ExtractSpanContext(
    const HTTPHeadersReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  auto iequals = [](string_view lhs, string_view rhs) {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  return ExtractSpanContext(carrier, trace_id, span_id, baggage, iequals);
}
}  // namespace lightstep

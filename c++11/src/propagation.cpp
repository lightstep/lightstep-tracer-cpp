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
// write_uint64
//------------------------------------------------------------------------------
static void write_uint64(std::ostream& out, uint64_t u) {
  out.write(reinterpret_cast<char*>(&u), sizeof(u));
}

//------------------------------------------------------------------------------
// read_uint64
//------------------------------------------------------------------------------
static uint64_t read_uint64(std::istream& in) {
  uint64_t u = 0;
  in.read(reinterpret_cast<char*>(&u), sizeof(u));
  if (!in.good()) {
    return 0;
  }
  return u;
}

//------------------------------------------------------------------------------
// write_string
//------------------------------------------------------------------------------
static void write_string(std::ostream& out, const std::string& s) {
  write_uint64(out, s.size());
  out.write(s.data(), s.size());
}

//------------------------------------------------------------------------------
// read_string
//------------------------------------------------------------------------------
static void read_string(std::istream& in, std::string& s) {
  auto length = read_uint64(in);
  s.resize(length);
  in.read(&s.front(), length);
}

//------------------------------------------------------------------------------
// inject_span_context
//------------------------------------------------------------------------------
expected<void> inject_span_context(
    std::ostream& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage) {
  // TODO(rnburn): Do we want to fiddle with carrier.exceptions() to ensure that
  // exceptions aren't thrown?
  write_uint64(carrier, trace_id);
  write_uint64(carrier, span_id);
  write_uint64(carrier, baggage.size());
  for (const auto& baggage_item : baggage) {
    write_string(carrier, baggage_item.first);
    write_string(carrier, baggage_item.second);
  }

  // Flush so that when we call carrier.good, we'll get an accurate view of the
  // error state.
  carrier.flush();
  if (!carrier.good()) {
    return make_unexpected(make_error_code(std::io_errc::stream));
  }

  return {};
}

expected<void> inject_span_context(
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
expected<bool> extract_span_context(
    std::istream& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) try {
  // istream::peek returns EOF if it's in an error state, so check for an error
  // state first before checking for an empty stream.
  if (!carrier.good()) {
    return make_unexpected(make_error_code(std::io_errc::stream));
  }

  // Check for the case when no span is encoded.
  if (carrier.peek() == EOF) {
    return false;
  }

  // TODO(rnburn): Do we want to fiddle with carrier.exceptions() to ensure that
  // exceptions aren't thrown?
  trace_id = read_uint64(carrier);
  span_id = read_uint64(carrier);
  auto num_baggage = read_uint64(carrier);
  baggage.reserve(num_baggage);
  std::string key, value;
  for (int i = 0; i < num_baggage; ++i) {
    read_string(carrier, key);
    read_string(carrier, value);
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
    return make_unexpected(make_error_code(std::io_errc::stream));
  }
  return true;
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}

template <class KeyCompare>
static expected<bool> extract_span_context(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage,
    KeyCompare key_compare) {
  int count = 0;
  auto result = carrier.ForeachKey([&](string_view key,
                                       string_view value) -> expected<void> {
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
               key_compare(string_view(key.data(), PrefixBaggage.size()),
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
  return true;
}

expected<bool> extract_span_context(
    const TextMapReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  return extract_span_context(carrier, trace_id, span_id, baggage,
                              std::equal_to<string_view>());
}

// HTTP header field names are case insensitive, so we need to ignore case when
// comparing against the OpenTracing field names.
//
// See https://stackoverflow.com/a/5259004/4447365
expected<bool> extract_span_context(
    const HTTPHeadersReader& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage) {
  auto iequals = [](string_view lhs, string_view rhs) {
    return lhs.length() == rhs.length() &&
           std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                      [](char a, char b) {
                        return std::tolower(a) == std::tolower(b);
                      });
  };
  return extract_span_context(carrier, trace_id, span_id, baggage, iequals);
}
}  // namespace lightstep

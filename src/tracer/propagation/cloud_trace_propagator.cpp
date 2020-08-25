#include "tracer/propagation/cloud_trace_propagator.h"

#include "common/hex_conversion.h"

#include "tracer/propagation/binary_propagation.h"
#include "tracer/propagation/utility.h"

const opentracing::string_view PropagationSingleKey = "x-cloud-trace-context";
const opentracing::string_view PrefixBaggage = "ot-baggage-";

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// InjectSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> CloudTracePropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageProtobufMap& /*baggage*/) const {
  return this->InjectSpanContextImpl(carrier, trace_context);
}

opentracing::expected<void> CloudTracePropagator::InjectSpanContext(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context, opentracing::string_view /*trace_state*/,
    const BaggageFlatMap& /*baggage*/) const {
  return this->InjectSpanContextImpl(carrier, trace_context);
}
//--------------------------------------------------------------------------------------------------
// ExtractSpanContext
//--------------------------------------------------------------------------------------------------
opentracing::expected<bool> CloudTracePropagator::ExtractSpanContext(
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
  opentracing::expected<bool> result;
  if (case_sensitive) {
    result = this->ExtractSpanContextImpl(carrier, trace_context,
                                    std::equal_to<opentracing::string_view>{}, baggage);
  } else {
    result = this->ExtractSpanContextImpl(carrier, trace_context, iequals, baggage);
  }
  if (!result || !*result) {
    return result;
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// InjectSpanContextImpl
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> CloudTracePropagator::InjectSpanContextImpl(
    const opentracing::TextMapWriter& carrier,
    const TraceContext& trace_context) const {
  std::array<char, CloudContextLength> buffer;
  auto data_length = this->SerializeCloudTrace(trace_context, buffer.data());
  return carrier.Set(PropagationSingleKey,
                  opentracing::string_view{buffer.data(), data_length});
}

//--------------------------------------------------------------------------------------------------
// ExtractSpanContextImpl
//--------------------------------------------------------------------------------------------------
template <class KeyCompare>
opentracing::expected<bool> CloudTracePropagator::ExtractSpanContextImpl(
    const opentracing::TextMapReader& carrier, TraceContext& trace_context, const KeyCompare& key_compare,
    BaggageProtobufMap& baggage) const {
      bool parent_header_found = false;
      auto result =
          carrier.ForeachKey([&](opentracing::string_view key,
                                 opentracing::string_view
                                     value) noexcept->opentracing::expected<void> {
            if (key_compare(key, PropagationSingleKey)) {
              auto was_successful = this->ParseCloudTrace(value, trace_context);
              if (!was_successful) {
                return opentracing::make_unexpected(was_successful.error());
              }
              parent_header_found = true;
            } else if (key.length() > PrefixBaggage.size() &&
                       key_compare(opentracing::string_view{key.data(),
                                                            PrefixBaggage.size()},
                                   PrefixBaggage)) {
              baggage.insert(BaggageProtobufMap::value_type(
                  ToLower(
                      opentracing::string_view{key.data() + PrefixBaggage.size(),
                                               key.size() - PrefixBaggage.size()}),
                  value));
            }
            return {};
          });
      if (!result) {
        return opentracing::make_unexpected(result.error());
      }
      return parent_header_found;
}

opentracing::expected<void> CloudTracePropagator::ParseCloudTrace(
    opentracing::string_view s, TraceContext& trace_context) const noexcept {
  if (s.size() < Num128BitHexDigits) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }
  size_t offset = 0;

  // default sampled to on (this comes from the ;o=1 part of
  // x-cloud-trace-context; if it is not set we will default to sampling
  // this request)
  trace_context.trace_flags = SetTraceFlag<SampledFlagMask>(trace_context.trace_flags, true);

  // trace-id
  auto error_maybe = NormalizedHexToUint128(
      opentracing::string_view{s.data() + offset, Num128BitHexDigits},
      trace_context.trace_id_high, trace_context.trace_id_low);
  if (!error_maybe) {
    return error_maybe;
  }

  offset += Num128BitHexDigits;
  if (s.size() - offset < 2) {
    // only a short form trace ID has been given (not a "trace id/span id")
    return {};
  }

  if (s[offset] != '/') {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }
  ++offset;

  std::array<char, Num64BitDecimalDigits + 1> parent_id;
  size_t i;
  for (i=0; i < Num64BitDecimalDigits; ++i) {
    if (offset == s.length() || std::isdigit(s[offset]) == 0) {
      break;
    }
    parent_id[i] = s[offset];
    ++offset;
  }
  parent_id[i] = '\0';

  // parent-id
  errno = 0;
  trace_context.parent_id = std::strtoull(parent_id.data(), nullptr, 10);
  if (errno == ERANGE) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::result_out_of_range));
  }

  if (s.size() - offset < 4) {
    // only a "trace ID/span ID" has been given (not a "trace id/span id;o=[0-1]")
    return {};
  }

  if(opentracing::string_view(s.begin() + offset, 3) != opentracing::string_view(";o=")) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }

  offset += 3;

  // trace-flags
  if (s[offset] == '0') {
    // don't sample
    trace_context.trace_flags = SetTraceFlag<SampledFlagMask>(trace_context.trace_flags, false);
  }

  return {};
}

size_t CloudTracePropagator::SerializeCloudTrace(const TraceContext& trace_context,
                           char* s) const noexcept {
  size_t offset = 0;
  // trace-id
  Uint64ToHex(trace_context.trace_id_high, s);
  offset += Num64BitHexDigits;
  Uint64ToHex(trace_context.trace_id_low, s + offset);
  offset += Num64BitHexDigits;

  offset += snprintf(
      s + offset, CloudContextLength - offset, "/%llu;o=%d",
      static_cast<unsigned long long>(trace_context.parent_id),
      IsTraceFlagSet<SampledFlagMask>(trace_context.trace_flags) ? 1 : 0);

  return offset;
}
}  // namespace lightstep

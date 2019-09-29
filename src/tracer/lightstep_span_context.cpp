#include "tracer/lightstep_span_context.h"

#include "common/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ToHexString
//--------------------------------------------------------------------------------------------------
static std::string ToHexString(uint64_t x) {
  std::array<char, Num64BitHexDigits> buffer;
  return Uint64ToHex(x, buffer.data());
}

//--------------------------------------------------------------------------------------------------
// ToTraceID
//--------------------------------------------------------------------------------------------------
std::string LightStepSpanContext::ToTraceID() const noexcept try {
  return ToHexString(this->trace_id());
} catch (const std::exception& /*e*/) {
  return {};
}

//--------------------------------------------------------------------------------------------------
// ToSpanID
//--------------------------------------------------------------------------------------------------
std::string LightStepSpanContext::ToSpanID() const noexcept try {
  return ToHexString(this->span_id());
} catch (const std::exception& /*e*/) {
  return {};
}

//------------------------------------------------------------------------------
// operator==
//------------------------------------------------------------------------------
bool operator==(const LightStepSpanContext& lhs,
                const LightStepSpanContext& rhs) {
  auto extract_baggage = [](const LightStepSpanContext& span_context) {
    std::unordered_map<std::string, std::string> baggage;
    span_context.ForeachBaggageItem(
        [&](const std::string& key, const std::string& value) {
          baggage.emplace(key, value);
          return true;
        });
    return baggage;
  };

  return lhs.trace_id() == rhs.trace_id() && lhs.span_id() == rhs.span_id() &&
         lhs.sampled() == rhs.sampled() &&
         extract_baggage(lhs) == extract_baggage(rhs);
}
}  // namespace lightstep

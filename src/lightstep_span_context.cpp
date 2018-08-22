#include "lightstep_span_context.h"

namespace lightstep {
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

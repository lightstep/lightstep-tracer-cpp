#include "tracer/lightstep_immutable_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
LightStepImmutableSpanContext::LightStepImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const std::unordered_map<std::string, std::string>& baggage)
    : trace_id_{trace_id}, span_id_{span_id}, sampled_{sampled} {
  for (auto& baggage_item : baggage) {
    baggage_.insert(
        BaggageMap::value_type(baggage_item.first, baggage_item.second));
  }
}

LightStepImmutableSpanContext::LightStepImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    BaggageMap&& baggage) noexcept
    : trace_id_{trace_id},
      span_id_{span_id},
      sampled_{sampled},
      baggage_{std::move(baggage)} {}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LightStepImmutableSpanContext::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  for (const auto& baggage_item : baggage_) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}
}  // namespace lightstep

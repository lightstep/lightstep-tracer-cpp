#include "tracer/legacy/legacy_immutable_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const std::unordered_map<std::string, std::string>& baggage)
    : LegacyImmutableSpanContext{0, trace_id, span_id, sampled, baggage} {}

LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id_high, uint64_t trace_id_low, uint64_t span_id,
    bool sampled, const std::unordered_map<std::string, std::string>& baggage)
    : trace_id_high_{trace_id_high},
      trace_id_{trace_id_low},
      span_id_{span_id},
      sampled_{sampled} {
  for (auto& baggage_item : baggage) {
    baggage_.insert(BaggageProtobufMap::value_type(baggage_item.first,
                                                   baggage_item.second));
  }
}

LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    BaggageProtobufMap&& baggage) noexcept
    : LegacyImmutableSpanContext{0, trace_id, span_id, sampled,
                                 std::move(baggage)} {}

LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id_high, uint64_t trace_id_low, uint64_t span_id,
    bool sampled, BaggageProtobufMap&& baggage) noexcept
    : trace_id_high_{trace_id_high},
      trace_id_{trace_id_low},
      span_id_{span_id},
      sampled_{sampled},
      baggage_{std::move(baggage)} {}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void LegacyImmutableSpanContext::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  for (const auto& baggage_item : baggage_) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}
}  // namespace lightstep

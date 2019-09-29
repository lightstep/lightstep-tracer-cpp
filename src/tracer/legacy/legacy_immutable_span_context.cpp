#include "tracer/legacy/legacy_immutable_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    const std::unordered_map<std::string, std::string>& baggage)
    : trace_id_{trace_id}, span_id_{span_id}, sampled_{sampled} {
  for (auto& baggage_item : baggage) {
    baggage_.insert(BaggageProtobufMap::value_type(baggage_item.first,
                                                   baggage_item.second));
  }
}

LegacyImmutableSpanContext::LegacyImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    BaggageProtobufMap&& baggage) noexcept
    : trace_id_{trace_id},
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

//--------------------------------------------------------------------------------------------------
// Clone
//--------------------------------------------------------------------------------------------------
std::unique_ptr<opentracing::SpanContext> LegacyImmutableSpanContext::Clone()
    const noexcept try {
  std::unique_ptr<opentracing::SpanContext> result{
      new LegacyImmutableSpanContext{trace_id_, span_id_, sampled_,
                                     BaggageProtobufMap{baggage_}}};
  return result;
} catch (const std::exception& /*e*/) {
  return nullptr;
}
}  // namespace lightstep

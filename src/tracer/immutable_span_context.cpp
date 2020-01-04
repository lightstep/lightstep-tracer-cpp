#include "tracer/immutable_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
ImmutableSpanContext::ImmutableSpanContext(
    uint64_t trace_id_high, uint64_t trace_id_low, uint64_t span_id,
    bool sampled, const std::unordered_map<std::string, std::string>& baggage)
    : ImmutableSpanContext{trace_id_high, trace_id_low, span_id, sampled,
                           BaggageProtobufMap{}} {
  for (auto& baggage_item : baggage) {
    baggage_.insert(BaggageProtobufMap::value_type(baggage_item.first,
                                                   baggage_item.second));
  }
}

ImmutableSpanContext::ImmutableSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    BaggageProtobufMap&& baggage) noexcept
    : ImmutableSpanContext{0, trace_id, span_id, sampled, std::move(baggage)} {}

ImmutableSpanContext::ImmutableSpanContext(
    uint64_t trace_id_high, uint64_t trace_id_low, uint64_t span_id,
    bool sampled, BaggageProtobufMap&& baggage) noexcept
    : trace_id_high_{trace_id_high},
      trace_id_low_{trace_id_low},
      span_id_{span_id},
      trace_flags_{SetTraceFlag<SampledFlagMask>(0, sampled)},
      baggage_{std::move(baggage)} {}

ImmutableSpanContext::ImmutableSpanContext(
    const TraceContext& trace_context, std::string&& trace_state,
    BaggageProtobufMap&& baggage) noexcept
    : trace_id_high_{trace_context.trace_id_high},
      trace_id_low_{trace_context.trace_id_low},
      span_id_{trace_context.parent_id},
      trace_flags_{trace_context.trace_flags},
      trace_state_{std::move(trace_state)},
      baggage_{std::move(baggage)} {}

//------------------------------------------------------------------------------
// ForeachBaggageItem
//------------------------------------------------------------------------------
void ImmutableSpanContext::ForeachBaggageItem(
    std::function<bool(const std::string& key, const std::string& value)> f)
    const {
  for (const auto& baggage_item : baggage_) {
    if (!f(baggage_item.first, baggage_item.second)) {
      return;
    }
  }
}
}  // namespace lightstep

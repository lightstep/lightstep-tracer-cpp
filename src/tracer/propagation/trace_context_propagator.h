#pragma once

#include "tracer/propagation/propagator.h"

namespace lightstep {
class TraceContextPropagator final : public Propagator {
 public:
  // Propagator
  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context, opentracing::string_view trace_state,
      const BaggageProtobufMap& baggage) const override;

  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context, opentracing::string_view trace_state,
      const BaggageFlatMap& baggage) const override;

  opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      TraceContext& trace_context, std::string& trace_state,
      BaggageProtobufMap& baggage) const override;
};
}  // namespace lightstep

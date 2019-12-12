#pragma once

#include "tracer/propagation/propagator.h"

namespace lightstep {
class TraceContextPropagator final : Propagator {
 public:
  // Propagator
  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageProtobufMap& baggage) const override;

  opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageFlatMap& baggage) const override;

  opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
      bool& sampled, BaggageProtobufMap& baggage) const override;

 private:
};
}  // namespace lightstep

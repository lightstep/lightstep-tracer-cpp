#pragma once

#include "tracer/propagation/propagator.h"

namespace lightstep {
/**
 * Propagator for Envoy's single-header format.
 */
class EnvoyPropagator final : public Propagator {
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

 private:
  template <class BaggageMap>
  opentracing::expected<void> InjectSpanContextImpl(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context, const BaggageMap& baggage) const;

  template <class KeyCompare>
  opentracing::expected<bool> ExtractSpanContextImpl(
      const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
      uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
      BaggageProtobufMap& baggage, const KeyCompare& key_compare) const;
};
}  // namespace lightstep

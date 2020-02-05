#pragma once

#include "tracer/propagation/propagator.h"

namespace lightstep {
/**
 * Propagator for multi-header formats.
 */
class MultiheaderPropagator final : public Propagator {
 public:
  MultiheaderPropagator(opentracing::string_view trace_id_key,
                        opentracing::string_view span_id_key,
                        opentracing::string_view sampled_key,
                        opentracing::string_view baggage_prefix,
                        bool supports_128bit) noexcept;

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
  opentracing::string_view trace_id_key_;
  opentracing::string_view span_id_key_;
  opentracing::string_view sampled_key_;
  opentracing::string_view baggage_prefix_;
  bool supports_128bit_;

  opentracing::expected<void> InjectSpanContextImpl(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context) const;

  template <class KeyCompare>
  opentracing::expected<bool> ExtractSpanContextImpl(
      const opentracing::TextMapReader& carrier, uint64_t& trace_id_high,
      uint64_t& trace_id_low, uint64_t& span_id, bool& sampled,
      BaggageProtobufMap& baggage, const KeyCompare& key_compare) const;
};
}  // namespace lightstep

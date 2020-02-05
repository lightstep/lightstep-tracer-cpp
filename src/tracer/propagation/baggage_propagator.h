#pragma once

#include "tracer/propagation/propagator.h"

namespace lightstep {
class BaggagePropagator final : public Propagator {
 public:
  explicit BaggagePropagator(opentracing::string_view baggage_prefix) noexcept;

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
      const opentracing::TextMapReader& /*carrier*/, bool /*case_sensitive*/,
      TraceContext& /*trace_context*/, std::string& /*trace_state*/,
      BaggageProtobufMap& /*baggage*/) const override {
    // Do nothing: baggage is extracted in the other propagators
    return false;
  }

 private:
  opentracing::string_view baggage_prefix_;

  template <class BaggageMap>
  opentracing::expected<void> InjectSpanContextImpl(
      const opentracing::TextMapWriter& carrier,
      const BaggageMap& baggage) const;
};
}  // namespace lightstep

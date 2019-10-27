#pragma once

#include "tracer/propagator.h"

namespace lightstep {
class BaggagePropagator final : public Propagator {
 public:
  explicit BaggagePropagator(opentracing::string_view baggage_prefix) noexcept;

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
      const opentracing::TextMapReader& /*carrier*/, bool /*case_sensitive*/,
      uint64_t& /*trace_id_high*/, uint64_t& /*trace_id_low*/,
      uint64_t& /*span_id*/, bool& /*sampled*/,
      BaggageProtobufMap& /*baggage*/) const {
    // Do nothing: baggage is extracted in the other propagators
    return true;
  }

 private:
  opentracing::string_view baggage_prefix_;

  template <class BaggageMap>
  opentracing::expected<void> InjectSpanContextImpl(
      const opentracing::TextMapWriter& carrier,
      const BaggageMap& baggage) const;
};
}  // namespace lightstep

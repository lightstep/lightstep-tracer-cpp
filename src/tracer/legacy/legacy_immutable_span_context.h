#pragma once

#include "tracer/lightstep_span_context.h"

namespace lightstep {
class LegacyImmutableSpanContext final : public LightStepSpanContext {
 public:
  LegacyImmutableSpanContext(
      uint64_t trace_id, uint64_t span_id, bool sampled,
      const std::unordered_map<std::string, std::string>& baggage);

  LegacyImmutableSpanContext(uint64_t trace_id, uint64_t span_id, bool sampled,
                             BaggageMap&& baggage) noexcept;

  uint64_t trace_id() const noexcept override { return trace_id_; }

  uint64_t span_id() const noexcept override { return span_id_; }

  bool sampled() const noexcept override { return sampled_; }

  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override;

  opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      std::ostream& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::TextMapWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::HTTPHeadersWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

 private:
  uint64_t trace_id_;
  uint64_t span_id_;
  bool sampled_;
  BaggageMap baggage_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    return InjectSpanContext(propagation_options, writer, trace_id_, span_id_,
                             sampled_, baggage_);
  }
};
}  // namespace lightstep

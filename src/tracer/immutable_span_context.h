#pragma once

#include "tracer/lightstep_span_context.h"
#include "tracer/propagation/trace_context.h"

namespace lightstep {
/**
 * Implements a span context with immutable values.
 */
class ImmutableSpanContext final : public LightStepSpanContext {
 public:
  ImmutableSpanContext(
      uint64_t trace_id_high, uint64_t trace_id_low, uint64_t span_id,
      bool sampled,
      const std::unordered_map<std::string, std::string>& baggage);

  ImmutableSpanContext(uint64_t trace_id, uint64_t span_id, bool sampled,
                       BaggageProtobufMap&& baggage) noexcept;

  ImmutableSpanContext(uint64_t trace_id_high, uint64_t trace_id_low,
                       uint64_t span_id, bool sampled,
                       BaggageProtobufMap&& baggage) noexcept;

  // LightStepSpanContext
  uint64_t trace_id_high() const noexcept override { return trace_context_.trace_id_high; }

  uint64_t trace_id_low() const noexcept override { return trace_context_.trace_id_low; }

  uint64_t span_id() const noexcept override {
    return trace_context_.parent_id;
  }

  bool sampled() const noexcept override {
    return IsTraceFlagSet<SampledFlagMask>(trace_context_.trace_flags);
  }

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
  TraceContext trace_context_;
  BaggageProtobufMap baggage_;
  std::string trace_state_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    return InjectSpanContext(propagation_options, writer, this->trace_id_high(),
                             this->trace_id_low(), this->span_id(),
                             this->sampled(), baggage_);
  }
};
}  // namespace lightstep

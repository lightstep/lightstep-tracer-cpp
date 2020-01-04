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

  ImmutableSpanContext(const TraceContext& trace_context,
                       std::string&& trace_state,
                       BaggageProtobufMap&& baggage) noexcept;

  // LightStepSpanContext
  uint64_t trace_id_high() const noexcept override { return trace_id_high_; }

  uint64_t trace_id_low() const noexcept override { return trace_id_low_; }

  uint64_t span_id() const noexcept override { return span_id_; }

  uint8_t trace_flags() const noexcept override { return trace_flags_; }

  opentracing::string_view trace_state() const noexcept override {
    return trace_state_;
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
  uint64_t trace_id_high_;
  uint64_t trace_id_low_;
  uint64_t span_id_;
  uint8_t trace_flags_;
  std::string trace_state_;
  BaggageProtobufMap baggage_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    TraceContext trace_context;
    trace_context.trace_id_high = trace_id_high_;
    trace_context.trace_id_low = trace_id_low_;
    trace_context.parent_id = span_id_;
    trace_context.trace_flags = trace_flags_;
    return InjectSpanContext(propagation_options, writer, trace_context,
                             trace_state_, baggage_);
  }
};
}  // namespace lightstep

#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "common/logger.h"
#include "lightstep-tracer-common/collector.pb.h"
#include "recorder/recorder.h"
#include "tracer/lightstep_span_context.h"

#include <opentracing/span.h>

namespace lightstep {
class LegacySpan final : public opentracing::Span, public LightStepSpanContext {
 public:
  LegacySpan(std::shared_ptr<const opentracing::Tracer>&& tracer,
             Logger& logger, Recorder& recorder,
             opentracing::string_view operation_name,
             const opentracing::StartSpanOptions& options);

  LegacySpan(const LegacySpan&) = delete;
  LegacySpan(LegacySpan&&) = delete;
  LegacySpan& operator=(const LegacySpan&) = delete;
  LegacySpan& operator=(LegacySpan&&) = delete;

  ~LegacySpan() override;

  void FinishWithOptions(
      const opentracing::FinishSpanOptions& options) noexcept override;

  void SetOperationName(opentracing::string_view name) noexcept override;

  void SetTag(opentracing::string_view key,
              const opentracing::Value& value) noexcept override;

  void SetBaggageItem(opentracing::string_view restricted_key,
                      opentracing::string_view value) noexcept override;

  std::string BaggageItem(opentracing::string_view restricted_key) const
      noexcept override;

  void Log(std::initializer_list<
           std::pair<opentracing::string_view, opentracing::Value>>
               fields) noexcept override;

  const opentracing::SpanContext& context() const noexcept override {
    return *this;
  }
  const opentracing::Tracer& tracer() const noexcept override {
    return *tracer_;
  }

  void ForeachBaggageItem(
      std::function<bool(const std::string& key, const std::string& value)> f)
      const override;

  uint64_t trace_id_high() const noexcept override { return trace_id_high_; }

  uint64_t trace_id_low() const noexcept override {
    return span_.span_context().trace_id();
  }

  uint64_t span_id() const noexcept override {
    return span_.span_context().span_id();
  }

  uint8_t trace_flags() const noexcept override;

  opentracing::string_view trace_state() const noexcept override {
    return trace_state_;
  }

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
  // Fields set in StartSpan() are not protected by a mutex.
  uint64_t trace_id_high_{0};
  uint8_t trace_flags_;
  collector::Span span_;
  std::string trace_state_;
  std::chrono::steady_clock::time_point start_steady_;
  mutable std::mutex mutex_;
  std::shared_ptr<const opentracing::Tracer> tracer_;
  Logger& logger_;
  Recorder& recorder_;

  std::atomic<bool> is_finished_{false};

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    auto& span_context = span_.span_context();
    TraceContext trace_context;
    trace_context.trace_id_high = trace_id_high_;
    trace_context.trace_id_low = span_context.trace_id();
    trace_context.parent_id = span_context.span_id();
    trace_context.trace_flags = trace_flags_;
    return InjectSpanContext(propagation_options, writer, trace_context,
                             trace_state_, span_context.baggage());
  }

  bool SetSpanReference(
      const std::pair<opentracing::SpanReferenceType,
                      const opentracing::SpanContext*>& reference,
      BaggageProtobufMap& baggage, collector::Reference& collector_reference,
      uint64_t& trace_id_high, uint64_t& trace_id);
};
}  // namespace lightstep

#pragma once

#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep_span_context.h"
#include "common/logger.h"
#include "recorder/recorder.h"

#include <opentracing/span.h>

#include <atomic>
#include <mutex>
#include <vector>

namespace lightstep {
class LightStepSpan final : public opentracing::Span,
                            public LightStepSpanContext {
 public:
  LightStepSpan(std::shared_ptr<const opentracing::Tracer>&& tracer,
                Logger& logger, Recorder& recorder,
                opentracing::string_view operation_name,
                const opentracing::StartSpanOptions& options);

  LightStepSpan(const LightStepSpan&) = delete;
  LightStepSpan(LightStepSpan&&) = delete;
  LightStepSpan& operator=(const LightStepSpan&) = delete;
  LightStepSpan& operator=(LightStepSpan&&) = delete;

  ~LightStepSpan() override;

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

  bool sampled() const noexcept override;

  uint64_t trace_id() const noexcept override {
    return span_.span_context().trace_id();
  }

  uint64_t span_id() const noexcept override {
    return span_.span_context().span_id();
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      std::ostream& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::TextMapWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

  virtual opentracing::expected<void> Inject(
      const PropagationOptions& propagation_options,
      const opentracing::HTTPHeadersWriter& writer) const override {
    return this->InjectImpl(propagation_options, writer);
  }

 private:
  // Fields set in StartSpan() are not protected by a mutex.
  collector::Span span_;
  std::chrono::steady_clock::time_point start_steady_;
  bool sampled_;
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
    return InjectSpanContext(propagation_options, writer,
                             span_context.trace_id(), span_context.span_id(),
                             sampled_, span_context.baggage());
  }
};
}  // namespace lightstep

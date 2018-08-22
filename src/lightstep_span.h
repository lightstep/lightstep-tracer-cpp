#pragma once

#include <opentracing/span.h>
#include <atomic>
#include <mutex>
#include <vector>
#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep_span_context.h"
#include "logger.h"
#include "recorder.h"

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
  mutable std::mutex mutex_;
  bool sampled_;
  std::shared_ptr<const opentracing::Tracer> tracer_;
  Logger& logger_;
  Recorder& recorder_;
  std::vector<collector::Reference> references_;
  std::chrono::steady_clock::time_point start_steady_;
  /* LightStepSpanContext span_context_; */

  std::atomic<bool> is_finished_{false};

  // Mutex protects tags_, logs_, and operation_name_.
  std::unordered_map<std::string, opentracing::Value> tags_;
  std::vector<collector::Log> logs_;

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

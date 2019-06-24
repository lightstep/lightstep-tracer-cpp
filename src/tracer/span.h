#pragma once

#include <memory>
#include <mutex>

#include "tracer/baggage_flat_map.h"
#include "tracer/lightstep_span_context.h"
#include "tracer/tracer_impl.h"

#include "common/serialization_chain.h"

#include <opentracing/span.h>
#include <google/protobuf/io/coded_stream.h>

namespace lightstep {
class Span final : public opentracing::Span,
                   public LightStepSpanContext {
 public:
  Span(std::shared_ptr<const TracerImpl>&& tracer,
       opentracing::string_view operation_name,
       const opentracing::StartSpanOptions& options);

  ~Span() noexcept override;

  // opentracing::Span
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

  // oopentracing::SpanContext
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

  // LightStepSpanContext
  bool sampled() const noexcept override;

  uint64_t trace_id() const noexcept override {
    return trace_id_;
  }

  uint64_t span_id() const noexcept override {
    return span_id_;
  }
 private:
  mutable std::mutex mutex_;
  std::unique_ptr<SerializationChain> serialization_chain_;
  google::protobuf::io::CodedOutputStream stream_;

  std::chrono::steady_clock::time_point start_steady_;
  std::atomic<bool> is_finished_{false};

  std::shared_ptr<const TracerImpl> tracer_;
  uint64_t trace_id_;
  uint64_t span_id_;
  BaggageFlatMap baggage_;
  bool sampled_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    (void)propagation_options;
    (void)writer;
#if 0
    std::lock_guard<std::mutex> lock_guard{mutex_};
    auto& span_context = span_.span_context();
    return InjectSpanContext(propagation_options, writer,
                             span_context.trace_id(), span_context.span_id(),
                             sampled_, span_context.baggage());
#endif
    return {};
  }

  bool SetSpanReference(
      const std::pair<opentracing::SpanReferenceType,
                      const opentracing::SpanContext*>& reference);
};
} // namespace lightstep

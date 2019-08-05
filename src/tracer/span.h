#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>

#include "common/serialization_chain.h"
#include "tracer/baggage_flat_map.h"
#include "tracer/lightstep_span_context.h"
#include "tracer/propagation.h"
#include "tracer/tracer_impl.h"

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/span.h>

namespace lightstep {
/**
 * LightStep's implementation of the OpenTracing Span class.
 */
class Span final : public BlockAllocatable,
                   public opentracing::Span,
                   public LightStepSpanContext {
 public:
  Span(std::shared_ptr<const TracerImpl>&& tracer,
       opentracing::string_view operation_name,
       const opentracing::StartSpanOptions& options);

  ~Span() noexcept override;

  void* operator new(size_t /*size*/) noexcept {
    assert(0 && "new cannot be used directly");
  }

  void operator delete(void* /*ptr*/) noexcept {
    // Span is freed from destructor
  }

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

  // opentracing::SpanContext
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

  uint64_t trace_id() const noexcept override { return trace_id_; }

  uint64_t span_id() const noexcept override { return span_id_; }

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
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return InjectSpanContext(propagation_options, writer, trace_id_, span_id_,
                             sampled_, baggage_);
  }

  bool SetSpanReference(
      const std::pair<opentracing::SpanReferenceType,
                      const opentracing::SpanContext*>& reference,
      uint64_t& trace_id);

  void FinishImpl(const opentracing::FinishSpanOptions& options) noexcept;
};
}  // namespace lightstep

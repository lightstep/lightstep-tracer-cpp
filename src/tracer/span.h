#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>

#include "common/chained_stream.h"
#include "common/spin_lock_mutex.h"
#include "tracer/baggage_flat_map.h"
#include "tracer/lightstep_span_context.h"
#include "tracer/propagation/propagation.h"
#include "tracer/tracer_impl.h"

#include <google/protobuf/io/coded_stream.h>
#include <opentracing/span.h>

namespace lightstep {
/**
 * LightStep's implementation of the OpenTracing Span class.
 */
class Span final : public opentracing::Span, public LightStepSpanContext {
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
  uint64_t trace_id_high() const noexcept override { return trace_id_high_; }

  uint64_t trace_id_low() const noexcept override { return trace_id_; }

  uint64_t span_id() const noexcept override { return span_id_; }

  uint8_t trace_flags() const noexcept override;

  opentracing::string_view trace_state() const noexcept override {
    return trace_state_;
  }

 private:
  // Profiling shows that even with no contention, locking and unlocking a
  // standard mutex represents a significant portion of the cost of
  // instrumentation.
  //
  // Even if there is contention, the span operations are cheap, so it makes
  // more sense to use a spin lock for this use case.
  mutable SpinLockMutex mutex_;

  std::unique_ptr<ChainedStream> chained_stream_;
  Fragment header_fragment_;
  google::protobuf::io::CodedOutputStream coded_stream_;

  std::chrono::steady_clock::time_point start_steady_;
  std::atomic<bool> is_finished_{false};

  std::shared_ptr<const TracerImpl> tracer_;
  uint64_t trace_id_high_{0};
  uint64_t trace_id_;
  uint64_t span_id_;
  uint8_t trace_flags_;
  BaggageFlatMap baggage_;
  std::string trace_state_;

  template <class Carrier>
  opentracing::expected<void> InjectImpl(
      const PropagationOptions& propagation_options, Carrier& writer) const {
    SpinLockGuard lock_guard{mutex_};
    TraceContext trace_context;
    trace_context.trace_id_high = trace_id_high_;
    trace_context.trace_id_low = trace_id_;
    trace_context.parent_id = span_id_;
    trace_context.trace_flags = trace_flags_;
    return InjectSpanContext(propagation_options, writer, trace_context,
                             trace_state_, baggage_);
  }

  bool SetSpanReference(
      const std::pair<opentracing::SpanReferenceType,
                      const opentracing::SpanContext*>& reference,
      uint64_t& trace_id_high, uint64_t& trace_id);

  void FinishImpl(const opentracing::FinishSpanOptions& options) noexcept;
};
}  // namespace lightstep

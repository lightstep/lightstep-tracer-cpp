#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/string_view.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <tuple>
#include <vector>
#include "lightstep_span.h"
#include "lightstep_span_context.h"
#include "propagation.h"
#include "recorder.h"
#include "utility.h"
using namespace opentracing;

namespace lightstep {
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    std::unique_ptr<Recorder>&& recorder);

//------------------------------------------------------------------------------
// LightStepTracerImpl
//------------------------------------------------------------------------------
namespace {
class LightStepTracerImpl
    : public LightStepTracer,
      public std::enable_shared_from_this<LightStepTracerImpl> {
 public:
  explicit LightStepTracerImpl(std::unique_ptr<Recorder>&& recorder)
      : recorder_(std::move(recorder)) {}

  std::unique_ptr<Span> StartSpanWithOptions(
      string_view operation_name, const StartSpanOptions& options) const
      noexcept override try {
    return std::unique_ptr<Span>(new LightStepSpan(
        shared_from_this(), *recorder_, operation_name, options));
  } catch (const std::bad_alloc&) {
    // Don't create a span if std::bad_alloc is thrown.
    return nullptr;
  }

  expected<void> Inject(const SpanContext& sc,
                        std::ostream& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<void> Inject(const SpanContext& sc,
                        const TextMapWriter& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<void> Inject(const SpanContext& sc,
                        const HTTPHeadersWriter& writer) const override {
    return InjectImpl(sc, writer);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      std::istream& reader) const override {
    return ExtractImpl(reader);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      const TextMapReader& reader) const override {
    return ExtractImpl(reader);
  }

  expected<std::unique_ptr<SpanContext>> Extract(
      const HTTPHeadersReader& reader) const override {
    return ExtractImpl(reader);
  }

  void Close() noexcept override {
    recorder_->FlushWithTimeout(std::chrono::hours(24));
  }

 private:
  std::unique_ptr<Recorder> recorder_;

  template <class Carrier>
  expected<void> InjectImpl(const SpanContext& sc, Carrier& writer) const {
    auto lightstep_span_context =
        dynamic_cast<const LightStepSpanContext*>(&sc);
    if (lightstep_span_context == nullptr) {
      return make_unexpected(invalid_span_context_error);
    }
    return lightstep_span_context->Inject(writer);
  }

  template <class Carrier>
  expected<std::unique_ptr<SpanContext>> ExtractImpl(Carrier& reader) const {
    auto lightstep_span_context = new (std::nothrow) LightStepSpanContext();
    std::unique_ptr<SpanContext> span_context(lightstep_span_context);
    if (!span_context) {
      return make_unexpected(make_error_code(std::errc::not_enough_memory));
    }
    auto result = lightstep_span_context->Extract(reader);
    if (!result) {
      return make_unexpected(result.error());
    }
    if (!*result) {
      span_context.reset();
    }
    return span_context;
  }
};
}  // anonymous namespace

//------------------------------------------------------------------------------
// GetTraceSpanIds
//------------------------------------------------------------------------------
expected<std::array<uint64_t, 2>> LightStepTracer::GetTraceSpanIds(
    const SpanContext& sc) const noexcept {
  auto lightstep_span_context = dynamic_cast<const LightStepSpanContext*>(&sc);
  if (lightstep_span_context == nullptr) {
    return make_unexpected(invalid_span_context_error);
  }
  std::array<uint64_t, 2> result = {
      {lightstep_span_context->trace_id(), lightstep_span_context->span_id()}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
expected<std::unique_ptr<SpanContext>> LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<SpanContext> result(
      new LightStepSpanContext(trace_id, span_id, std::move(baggage)));
  return result;
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    std::unique_ptr<Recorder>&& recorder) {
  if (!recorder) {
    return nullptr;
  }
  return std::shared_ptr<opentracing::Tracer>(
      new (std::nothrow) LightStepTracerImpl(std::move(recorder)));
}

std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    const LightStepTracerOptions& options) {
  return MakeLightStepTracer(make_lightstep_recorder(options));
}
}  // namespace lightstep

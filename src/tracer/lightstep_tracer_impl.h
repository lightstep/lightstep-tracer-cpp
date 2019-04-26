#pragma once

#include <memory>
#include "common/logger.h"
#include "recorder/recorder.h"
#include "tracer/propagation.h"

namespace lightstep {
class LightStepTracerImpl final
    : public LightStepTracer,
      public std::enable_shared_from_this<LightStepTracerImpl> {
 public:
  LightStepTracerImpl(const PropagationOptions& propagation_options,
                      std::unique_ptr<Recorder>&& recorder) noexcept;

  LightStepTracerImpl(std::shared_ptr<Logger> logger,
                      const PropagationOptions& propagation_options,
                      std::unique_ptr<Recorder>&& recorder) noexcept;

  std::unique_ptr<opentracing::Span> StartSpanWithOptions(
      opentracing::string_view operation_name,
      const opentracing::StartSpanOptions& options) const noexcept override;

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& span_context,
      std::ostream& writer) const override;

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& span_context,
      const opentracing::TextMapWriter& writer) const override;

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& span_context,
      const opentracing::HTTPHeadersWriter& writer) const override;

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      std::istream& reader) const override;

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::TextMapReader& reader) const override;

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::HTTPHeadersReader& reader) const override;

  bool Flush() noexcept override;

  bool FlushWithTimeout(std::chrono::system_clock::duration timeout) noexcept override;

  void Close() noexcept override;

 private:
  std::shared_ptr<Logger> logger_;
  PropagationOptions propagation_options_;
  std::unique_ptr<Recorder> recorder_;
};
}  // namespace lightstep

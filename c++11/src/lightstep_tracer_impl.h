#pragma once

#include <lightstep/spdlog/logger.h>
#include <memory>
#include "recorder.h"

namespace lightstep {
class LightStepTracerImpl
    : public LightStepTracer,
      public std::enable_shared_from_this<LightStepTracerImpl> {
 public:
  LightStepTracerImpl(std::unique_ptr<Recorder>&& recorder) noexcept;

  LightStepTracerImpl(std::shared_ptr<spdlog::logger> logger,
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

  void Flush() noexcept override;

  void Close() noexcept override;

 private:
  std::shared_ptr<spdlog::logger> logger_;
  std::unique_ptr<Recorder> recorder_;
};
}  // namespace lightstep

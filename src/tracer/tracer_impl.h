#pragma once

#include <memory>

#include "common/logger.h"
#include "recorder/recorder.h"
#include "tracer/allocator_manager.h"
#include "tracer/propagation.h"

namespace lightstep {
/**
 * LightStep's implementation of the opentracing Tracer class.
 */
class TracerImpl final : public LightStepTracer,
                         public std::enable_shared_from_this<TracerImpl> {
 public:
  TracerImpl(const PropagationOptions& propagation_options,
             std::unique_ptr<Recorder>&& recorder) noexcept;

  TracerImpl(std::shared_ptr<Logger> logger,
             const PropagationOptions& propagation_options,
             std::unique_ptr<Recorder>&& recorder) noexcept;

  /**
   * @return the associated Logger.
   */
  Logger& logger() const noexcept { return *logger_; }

  AllocatorManager& allocator_manager() const noexcept {
    return allocator_manager_;
  }

  /**
   * @return the associated Recorder
   */
  Recorder& recorder() const noexcept { return *recorder_; }

  // opentracing::Span
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

  void Close() noexcept override;

  // LightStepTracer
  bool Flush() noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  std::shared_ptr<Logger> logger_;
  PropagationOptions propagation_options_;
  mutable AllocatorManager allocator_manager_;
  std::unique_ptr<Recorder> recorder_;
};
}  // namespace lightstep

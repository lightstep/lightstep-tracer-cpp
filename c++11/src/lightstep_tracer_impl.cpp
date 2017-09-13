#include "lightstep_tracer_impl.h"
#include "lightstep_span.h"
#include "lightstep_span_context.h"

namespace lightstep {

//------------------------------------------------------------------------------
// InjectImpl
//------------------------------------------------------------------------------
template <class Carrier>
static opentracing::expected<void> InjectImpl(
    const opentracing::SpanContext& span_context, Carrier& writer) {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  return lightstep_span_context->Inject(writer);
}

//------------------------------------------------------------------------------
// ExtractImpl
//------------------------------------------------------------------------------
template <class Carrier>
opentracing::expected<std::unique_ptr<opentracing::SpanContext>> ExtractImpl(
    Carrier& reader) {
  auto lightstep_span_context = new (std::nothrow) LightStepSpanContext{};
  std::unique_ptr<opentracing::SpanContext> span_context(
      lightstep_span_context);
  if (!span_context) {
    return opentracing::make_unexpected(
        make_error_code(std::errc::not_enough_memory));
  }
  auto result = lightstep_span_context->Extract(reader);
  if (!result) {
    return opentracing::make_unexpected(result.error());
  }
  if (!*result) {
    span_context.reset();
  }
  return std::move(span_context);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepTracerImpl::LightStepTracerImpl(
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::make_shared<Logger>()}, recorder_{std::move(recorder)} {}

LightStepTracerImpl::LightStepTracerImpl(
    std::shared_ptr<Logger> logger,
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::move(logger)}, recorder_{std::move(recorder)} {}

//------------------------------------------------------------------------------
// StartSpanWithOptions
//------------------------------------------------------------------------------
std::unique_ptr<opentracing::Span> LightStepTracerImpl::StartSpanWithOptions(
    opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options) const noexcept try {
  return std::unique_ptr<opentracing::Span>{new LightStepSpan{
      shared_from_this(), *logger_, *recorder_, operation_name, options}};
} catch (const std::exception& e) {
  logger_->Error("StartSpanWithOptions failed: ", e.what());
  return nullptr;
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context, std::ostream& writer) const {
  return InjectImpl(span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::TextMapWriter& writer) const {
  return InjectImpl(span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::HTTPHeadersWriter& writer) const {
  return InjectImpl(span_context, writer);
}

//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(std::istream& reader) const {
  return ExtractImpl(reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(const opentracing::TextMapReader& reader) const {
  return ExtractImpl(reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(
    const opentracing::HTTPHeadersReader& reader) const {
  return ExtractImpl(reader);
}

//------------------------------------------------------------------------------
// Flush
//------------------------------------------------------------------------------
bool LightStepTracerImpl::Flush() noexcept {
  return recorder_->FlushWithTimeout(std::chrono::hours(24));
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void LightStepTracerImpl::Close() noexcept { Flush(); }
}  // namespace lightstep

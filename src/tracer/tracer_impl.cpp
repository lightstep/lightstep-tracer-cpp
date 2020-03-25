#include "tracer/tracer_impl.h"

#include "tracer/immutable_span_context.h"
#include "tracer/span.h"

namespace lightstep {
const auto DefaultFlushTimeout = std::chrono::seconds{10};

//------------------------------------------------------------------------------
// InjectImpl
//------------------------------------------------------------------------------
template <class Carrier>
static opentracing::expected<void> InjectImpl(
    const PropagationOptions& propagation_options,
    const opentracing::SpanContext& span_context, Carrier& writer) {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  return lightstep_span_context->Inject(propagation_options, writer);
}

//------------------------------------------------------------------------------
// ExtractImpl
//------------------------------------------------------------------------------
template <class Carrier>
static opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
ExtractImpl(const PropagationOptions& propagation_options,
            Carrier& reader) try {
  TraceContext trace_context;
  std::string trace_state;
  BaggageProtobufMap baggage;

  auto extract_maybe = ExtractSpanContext(propagation_options, reader,
                                          trace_context, trace_state, baggage);

  if (!extract_maybe) {
    return opentracing::make_unexpected(extract_maybe.error());
  }
  if (!*extract_maybe) {
    return std::unique_ptr<opentracing::SpanContext>{nullptr};
  }
  std::unique_ptr<opentracing::SpanContext> result{new ImmutableSpanContext{
      trace_context, std::move(trace_state), std::move(baggage)}};
  return result;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      make_error_code(std::errc::not_enough_memory));
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
TracerImpl::TracerImpl(PropagationOptions&& propagation_options,
                       std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::make_shared<Logger>()},
      propagation_options_{std::move(propagation_options)},
      recorder_{std::move(recorder)} {}

TracerImpl::TracerImpl(std::shared_ptr<Logger> logger,
                       PropagationOptions&& propagation_options,
                       std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::move(logger)},
      propagation_options_{std::move(propagation_options)},
      recorder_{std::move(recorder)} {}

//------------------------------------------------------------------------------
// StartSpanWithOptions
//------------------------------------------------------------------------------
std::unique_ptr<opentracing::Span> TracerImpl::StartSpanWithOptions(
    opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options) const noexcept try {
  return std::unique_ptr<opentracing::Span>{
      new Span{shared_from_this(), operation_name, options}};
} catch (const std::exception& e) {
  logger_->Error("StartSpanWithOptions failed: ", e.what());
  return nullptr;
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
opentracing::expected<void> TracerImpl::Inject(
    const opentracing::SpanContext& span_context, std::ostream& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> TracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::TextMapWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> TracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::HTTPHeadersWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
TracerImpl::Extract(std::istream& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
TracerImpl::Extract(const opentracing::TextMapReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
TracerImpl::Extract(const opentracing::HTTPHeadersReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

//------------------------------------------------------------------------------
// Flush
//------------------------------------------------------------------------------
bool TracerImpl::Flush() noexcept {
  return recorder_->FlushWithTimeout(DefaultFlushTimeout);
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool TracerImpl::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {
  return recorder_->FlushWithTimeout(timeout);
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void TracerImpl::Close() noexcept {
  auto t1 = std::chrono::steady_clock::now();
  if (!recorder_->FlushWithTimeout(DefaultFlushTimeout)) {
    return;
  }
  auto delta = std::chrono::steady_clock::now() - t1;
  if (delta >= DefaultFlushTimeout) {
    return;
  }
  auto timeout =
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          DefaultFlushTimeout) -
      delta;
  recorder_->ShutdownWithTimeout(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(timeout));
}
}  // namespace lightstep

#include "impl.h"
#include "options.h"
#include "span.h"
#include "tracer.h"

namespace lightstep {

Span::Span(std::unique_ptr<SpanImpl> impl)
  :impl_(std::move(impl)) { }

Span& Span::SetOperationName(const std::string& name) {
  if (impl_) {
    impl_->SetOperationName(name);
  }
  return *this;
}

Span& Span::SetTag(const std::string& key, const Value& value) {
  if (impl_) {
    impl_->SetTag(key, value);
  }
  return *this;
}

Span& Span::SetBaggageItem(const std::string& restricted_key,
			   const std::string& value) {
  if (impl_) {
    impl_->SetBaggageItem(restricted_key, value);
  }
  return *this;
}

std::string Span::BaggageItem(const std::string& restricted_key) {
  if (!impl_) return "";
  return impl_->BaggageItem(restricted_key);
}

void Span::Finish(std::initializer_list<FinishSpanOption> opts) {
  if (!impl_) return;
  impl_->FinishSpan(opts);  
}

Span StartSpan(const std::string& operation_name, std::initializer_list<StartSpanOption> opts) {
  return Tracer::Global().StartSpan(operation_name, opts);
}

Tracer Span::tracer() const {
  if (!impl_) return Tracer(nullptr);
  return Tracer(impl_->tracer_);
}

SpanContext Span::context() const {
  return SpanContext(impl_);
}

uint64_t SpanContext::trace_id() const { return owner_->context_.trace_id; }
uint64_t SpanContext::span_id() const { return owner_->context_.span_id; }
uint64_t SpanContext::parent_span_id() const { return owner_->context_.parent_span_id; }
bool SpanContext::sampled() const { return owner_->context_.sampled; }

void SpanContext::ForeachBaggageItem(std::function<bool(const std::string& key, const std::string& value)> f) const {
  for (const auto& bi : owner_->context_.baggage) {
    if (!f(bi.first, bi.second)) {
      return;
    }
  }
}

} // namespace lightstep

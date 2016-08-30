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

void Span::Finish(SpanFinishOptions opts) {
  if (!impl_) return;
  impl_->FinishSpan(opts);  
}

Span StartSpan(const std::string& operation_name, SpanStartOptions opts) {
  return Tracer::Global().StartSpan(operation_name, opts);
}

Tracer Span::tracer() const {
  if (!impl_) return Tracer(nullptr);
  return Tracer(impl_->tracer_);
}

SpanContext Span::context() const {
  return SpanContext(impl_);
}

const ContextImpl* SpanContext::ctx() const {
  if (span_ctx_ != nullptr) {
    return &span_ctx_->context_;
  } else if (impl_ctx_ != nullptr) {
    return impl_ctx_.get();
  } else {
    return nullptr;
  }
}

uint64_t SpanContext::trace_id() const {
  if (const ContextImpl* impl = ctx()) {
    return impl->trace_id;
  }
  return 0;
}
uint64_t SpanContext::span_id() const {
  if (const ContextImpl* impl = ctx()) {
    return impl->span_id;
  }
  return 0;
}

void SpanContext::ForeachBaggageItem(std::function<bool(const std::string& key,
							const std::string& value)> f) const {
  if (const ContextImpl* impl = ctx()) {
    impl->foreachBaggageItem(f);
  }
}

} // namespace lightstep

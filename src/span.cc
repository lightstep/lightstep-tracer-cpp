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

void Span::Finish() {
  if (!impl_) return;
  impl_->FinishSpan(FinishSpanOptions());
}

void Span::FinishWithOptions(const FinishSpanOptions& fopts) {
  if (!impl_) return;
  impl_->FinishSpan(fopts);
}

Span StartSpan(const std::string& operation_name) {
  return Tracer::Global().StartSpan(operation_name);
}

Span StartChildSpan(Span parent, const std::string& operation_name) {
  return Tracer::Global().StartSpanWithOptions(StartSpanOptions().
					       SetOperationName(operation_name).
					       SetParent(parent));
}

Tracer Span::tracer() const {
  if (!impl_) return Tracer(nullptr);
  return Tracer(nullptr);
}
  
} // namespace lightstep

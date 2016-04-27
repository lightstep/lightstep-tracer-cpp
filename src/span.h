// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_SPAN_H__
#define __LIGHTSTEP_SPAN_H__

#include "impl.h"
#include "value.h"

#include <memory>

namespace lightstep {

class FinishOptions;
class Tracer;

class Span {
public:
  Span() { }

  explicit Span(std::unique_ptr<SpanImpl> impl) :impl_(std::move(impl)) { }

  Span& SetOperationName(const std::string& name) {
    if (impl_) {
      impl_->SetOperationName(name);
    }
    return *this;
  }

  Span& SetTag(const std::string& key, const Value& value) {
    if (impl_) {
      impl_->SetTag(key, value);
    }
    return *this;
  }

  void Finish() {
    if (!impl_) return;

  }

  void FinishWithOptions(const FinishOptions& fopts) {
    if (!impl_) return;

  }

  // TODO LogData

  Span& SetBaggageItem(const std::string& restricted_key,
		       const std::string& value) {
    if (impl_) {
    }
    return *this;
  }

  std::string BaggageItem(const std::string& restricted_key) {
    if (!impl_) return "";
    return "";
  }

  // Gets the Tracer associated with this Span.
  Tracer tracer();

private:
  std::shared_ptr<SpanImpl> impl_;
};

Span StartSpan(const std::string& operation_name);
Span StartChildSpan(Span parent, const std::string& operation_name);

} // namespace lightstep

#endif // __LIGHTSTEP_SPAN_H__

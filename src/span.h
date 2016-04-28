// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_SPAN_H__
#define __LIGHTSTEP_SPAN_H__

#include "value.h"

#include <memory>

namespace lightstep {

struct FinishSpanOptions;
class Tracer;
class SpanImpl;

class Span {
public:
  Span() { }
 
  explicit Span(std::unique_ptr<SpanImpl> impl);

  Span& SetOperationName(const std::string& name);

  Span& SetTag(const std::string& key, const Value& value);

  void Finish();
  void FinishWithOptions(const FinishSpanOptions& fopts);

  // TODO LogData
  // TODO error flag

  Span& SetBaggageItem(const std::string& restricted_key,
		       const std::string& value);

  std::string BaggageItem(const std::string& restricted_key);

  // Gets the Tracer associated with this Span.
  Tracer tracer() const;

  // Get the implementation object.
  std::shared_ptr<SpanImpl> impl() const { return impl_; }

private:
  std::shared_ptr<SpanImpl> impl_;
};

Span StartSpan(const std::string& operation_name);
Span StartChildSpan(Span parent, const std::string& operation_name);

} // namespace lightstep

#endif // __LIGHTSTEP_SPAN_H__

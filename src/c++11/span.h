// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_SPAN_H__
#define __LIGHTSTEP_SPAN_H__

#include "options.h"
#include "propagation.h"
#include "value.h"

#include <memory>

namespace lightstep {

class Tracer;
class SpanImpl;

// Span is a handle to an active (started), inactive (started and
// finished), or no-op span.
class Span {
public:
  // Constructor for a no-op span.
  Span() { }

  // For the Tracer implementation to construct real spans.
  explicit Span(std::unique_ptr<SpanImpl> impl);

  // SetOperationName may be called prior to Finish.
  Span& SetOperationName(const std::string& name);

  // SetTag may be called prior to Finish.
  Span& SetTag(const std::string& key, const Value& value);

  // Finish the span with default options.
  void Finish(SpanFinishOptions opts = {});

  // SetBaggageItem may be be called prior to Finish.
  Span& SetBaggageItem(const std::string& restricted_key,
		       const std::string& value);

  // BaggageItem may be called before or after Finish.
  std::string BaggageItem(const std::string& restricted_key);

  // Gets the Tracer associated with this Span.
  Tracer tracer() const;

  // Gets an immutable reference to this Span's context.
  SpanContext context() const;

  // Get the implementation object.
  std::shared_ptr<SpanImpl> impl() const { return impl_; }

  // TODO LogData not implemented
  // TODO Set error flag not implemented

private:
  std::shared_ptr<SpanImpl> impl_;
};

// Convenience method for starting a span using the Global tracer.
Span StartSpan(const std::string& operation_name, SpanStartOptions opts = {});

} // namespace lightstep

#endif // __LIGHTSTEP_SPAN_H__

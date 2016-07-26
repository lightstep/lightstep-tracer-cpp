// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_SPAN_H__
#define __LIGHTSTEP_SPAN_H__

#include "value.h"

#include <memory>

namespace lightstep {

class SpanStartOption;
class SpanFinishOption;
class Tracer;
class SpanImpl;

class SpanContext {
public:

  // TODO opentracing to possibly std::string BaggageItem(const std::string& key);
  // See https://github.com/opentracing/opentracing.github.io/issues/106

  uint64_t trace_id() const;
  uint64_t span_id() const;
  uint64_t parent_span_id() const;
  bool sampled() const;

  void ForeachBaggageItem(std::function<bool(const std::string& key, const std::string& value)> f) const;

private:
  friend class SpanImpl;
  friend class Span;

  explicit SpanContext(std::shared_ptr<SpanImpl> span) : owner_(span) { }

  std::shared_ptr<SpanImpl> owner_;
};

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
  void Finish(std::initializer_list<SpanFinishOption> opts = {});

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
Span StartSpan(const std::string& operation_name, std::initializer_list<SpanStartOption> opts = {});

} // namespace lightstep

#endif // __LIGHTSTEP_SPAN_H__

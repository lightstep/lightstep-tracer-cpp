// -*- Mode: C++ -*-
#include "lightstep_thrift/lightstep_constants.h"
#include "lightstep_thrift/lightstep_types.h"

#include "options.h"
#include "span.h"
#include "value.h"

#include <memory>

#ifndef __LIGHTSTEP_TRACER_H__
#define __LIGHTSTEP_TRACER_H__

namespace lightstep {

class TracerImpl;

typedef std::shared_ptr<TracerImpl> ImplPtr;

// Tracer is a handle to a TracerImpl or acts as a No-Op.
class Tracer {
 public:
  explicit Tracer(const ImplPtr &impl) : impl_(impl) { }

  // Constructs a No-Op tracer handle. implementation.
  explicit Tracer(std::nullptr_t) { }

  Span StartSpan(const std::string& operation_name);

  Span StartSpanWithOptions(const StartSpanOptions& options);

  // GlobalTracer returns the global tracer.
  static Tracer Global();

  // InitGlobalTracer sets the global tracer pointer, returns the
  // former global tracer value.
  static Tracer InitGlobal(Tracer);
  
  // Get the implementation object.
  ImplPtr impl() const { return impl_; }

 private:
  ImplPtr impl_;
};

Tracer NewTracer(const TracerOptions& options);
  
}  // namespace lightstep

#endif // __LIGHTSTEP_TRACER_H__

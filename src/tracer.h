// -*- Mode: C++ -*-
#include "lightstep_thrift/lightstep_constants.h"
#include "lightstep_thrift/lightstep_types.h"

#include "impl.h"
#include "options.h"
#include "span.h"
#include "value.h"

#include <memory>

#ifndef __LIGHTSTEP_TRACER_H__
#define __LIGHTSTEP_TRACER_H__

namespace lightstep {

class Tracer {
 public:
  Tracer(std::shared_ptr<TracerImpl> *impl) {
    if (!impl) return;
    impl_ = *impl;
  }

  Span StartSpan(const std::string& operation_name) {
    if (!impl_) return Span();
    return Span();
  }

  Span StartSpanWithOptions(const StartSpanOptions& options) {
    if (!impl_) return Span();
    return Span();
  }

  // GlobalTracer returns the global tracer.
  static Tracer Global();

  // InitGlobalTracer sets the global tracer pointer, returns the
  // former global tracer value.
  static Tracer InitGlobal(Tracer);
  
 private:
  std::shared_ptr<TracerImpl> impl_;
};
  
}  // namespace lightstep

#endif // __LIGHTSTEP_TRACER_H__

// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_SPAN_H__
#define __LIGHTSTEP_SPAN_H__

#include "impl.h"

#include <memory>

namespace lightstep {

class Span {
public:
  Span() { }

  explicit Span(std::unique_ptr<SpanImpl> impl) :impl_(std::move(impl)) { }

  void Finish() {
    if (!impl_) return;
  }

private:
  std::shared_ptr<SpanImpl> impl_;
};

Span StartSpan(const std::string& operation_name);
Span StartChildSpan(Span parent, const std::string& operation_name);

} // namespace lightstep

#endif // __LIGHTSTEP_SPAN_H__

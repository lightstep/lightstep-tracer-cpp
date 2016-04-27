// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_IMPL_H__
#define __LIGHTSTEP_IMPL_H__

#include <memory>
#include "value.h"

namespace lightstep {

class SpanImpl;
class StartSpanOptions;

class TracerImpl {
 public:
  std::unique_ptr<SpanImpl> StartSpan(const StartSpanOptions& options);

 private:
};

class SpanImpl {
 public:
  void SetOperationName(const std::string& name);
  void SetTag(const std::string& key, const Value& value);

 private:
  std::shared_ptr<TracerImpl> tracer_impl_;
};

} // namespace lightstep

#endif

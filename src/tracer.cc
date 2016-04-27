#include <atomic>
#include <memory>
#include <mutex>

#include "tracer.h"

namespace lightstep {
namespace {

typedef std::shared_ptr<TracerImpl> SharedTracerImpl;

std::atomic<SharedTracerImpl*> global_tracer;

}  // namespace

Tracer::Tracer(std::shared_ptr<TracerImpl> *impl) {
  if (!impl) return;
  impl_ = *impl;
}

Tracer Tracer::Global() {
  return Tracer(global_tracer.load());
}

Tracer Tracer::InitGlobal(Tracer newt) {
  auto shimpl = global_tracer.exchange(new SharedTracerImpl(newt.impl_));
  Tracer tracer(shimpl);      // This takes a reference (if not null)
  if (shimpl) delete shimpl;  // Destroys the global reference
  return tracer;
}

Span Tracer::StartSpan(const std::string& operation_name) {
  auto opts = StartSpanOptions().SetOperationName(operation_name);
  return StartSpanWithOptions(opts);
}

Span Tracer::StartSpanWithOptions(const StartSpanOptions& options) {
  if (!impl_) return Span();
  return Span(impl_->StartSpan(options));
}

}  // namespace lightstep

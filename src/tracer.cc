#include <atomic>
#include <memory>

#include "impl.h"
#include "tracer.h"

namespace lightstep {
namespace {

std::atomic<ImplPtr*> global_tracer;

}  // namespace

Tracer Tracer::Global() {
  ImplPtr *ptr = global_tracer.load();
  if (ptr == nullptr) {
    return Tracer(nullptr);
  }
  // Note: there is an intentional race here.  InitGlobal releases the
  // shared pointer reference without synchronization.
  return Tracer(*ptr);
}

Tracer Tracer::InitGlobal(Tracer newt) {
  ImplPtr *newptr = new ImplPtr(newt.impl_);
  if (auto oldptr = global_tracer.exchange(newptr)) {
    delete oldptr;
  }
  return Tracer(*newptr);
}

Span Tracer::StartSpan(const std::string& operation_name) {
  return StartSpanWithOptions(StartSpanOptions().
			      SetOperationName(operation_name));
}

Span Tracer::StartSpanWithOptions(const StartSpanOptions& options) {
  if (!impl_) return Span();
  return Span(impl_->StartSpan(impl_, options));
}

Tracer NewTracer(const TracerOptions& options) {
  ImplPtr impl(new TracerImpl(options));
  return Tracer(impl);
}

}  // namespace lightstep

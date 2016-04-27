#include <atomic>
#include <memory>
#include <mutex>

#include "tracer.h"

namespace lightstep {
namespace {

typedef std::shared_ptr<TracerImpl> SharedTracerImpl;

std::atomic<SharedTracerImpl*> global_tracer;

}  // namespace

Tracer Tracer::Global() {
  return Tracer(global_tracer.load());
}

Tracer Tracer::InitGlobal(Tracer newt) {
  return global_tracer.exchange(new SharedTracerImpl(newt.impl_));
}

}  // namespace lightstep

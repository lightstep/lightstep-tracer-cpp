#include <lightstep/tracer.h>
#include <opentracing/noop.h>

namespace lightstep {
std::shared_ptr<opentracing::Tracer> make_lightstep_tracer() {
  return opentracing::make_noop_tracer();
}
} // namespace lightstep

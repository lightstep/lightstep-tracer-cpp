#ifndef LIGHTSTEP_TRACER_H
#define LIGHTSTEP_TRACER_H

#include <opentracing/tracer.h>
#include <memory>

namespace lightstep {
std::shared_ptr<opentracing::Tracer> make_lightstep_tracer();
} // namespace lightstep

#endif // LIGHTSTEP_TRACER_H

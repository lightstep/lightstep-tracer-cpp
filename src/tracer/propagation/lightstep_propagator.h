#pragma once

#include <memory>

#include "tracer/propagation/propagator.h"

#include <opentracing/string_view.h>

namespace lightstep {
extern const opentracing::string_view PrefixBaggage;

std::unique_ptr<Propagator> MakeLightStepPropagator();
}  // namespace lightstep

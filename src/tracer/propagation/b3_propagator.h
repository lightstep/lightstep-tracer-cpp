#pragma once

#include <memory>

#include "tracer/propagation/propagator.h"

namespace lightstep {
std::unique_ptr<Propagator> MakeB3Propagator();
}  // namespace lightstep

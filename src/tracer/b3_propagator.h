#pragma once

#include <memory>

#include "propagator.h"

#include <opentracing/string_view.h>

namespace lightstep {
std::unique_ptr<Propagator> MakeB3Propagator();
}  // namespace lightstep

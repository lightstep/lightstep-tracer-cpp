#pragma once

#include <memory>
#include <vector>

#include "tracer/propagation/propagator.h"

#include <lightstep/tracer.h>

namespace lightstep {
struct PropagationOptions {
  std::vector<std::unique_ptr<Propagator>> inject_propagators;
  std::vector<std::unique_ptr<Propagator>> extract_propagators;
};

void SetInjectExtractPropagationModes(
    const std::vector<PropagationMode>& propagation_modes,
    std::vector<PropagationMode>& inject_propagation_modes,
    std::vector<PropagationMode>& extract_propagation_modes);

PropagationOptions MakePropagationOptions(
    const LightStepTracerOptions& options);
}  // namespace lightstep

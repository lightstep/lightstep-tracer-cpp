#pragma once

#include <memory>
#include <vector>

#include "tracer/propagator.h"

#include <lightstep/tracer.h>

namespace lightstep {
struct PropagationOptions {
  PropagationMode propagation_mode;
  bool use_single_key = false;
  std::vector<std::unique_ptr<Propagator>> inject_propagators;
  std::vector<std::unique_ptr<Propagator>> extract_propagators;
};

void SetInjectExtractPropagationModes(
    const std::vector<PropagationMode>& propagation_modes,
    std::vector<PropagationMode>& inject_propagation_modes,
    std::vector<PropagationMode>& extract_propagation_modes);

PropagationOptions MakePropagationOptions(
    const std::vector<PropagationMode>& propagation_modes);

PropagationOptions MakePropagationOptions(
    const LightStepTracerOptions& options);
}  // namespace lightstep

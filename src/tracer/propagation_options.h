#pragma once

#include <vector>

#include <lightstep/tracer.h>

namespace lightstep {
struct PropagationOptions {
  PropagationMode propagation_mode;
  bool use_single_key = false;
};

void SetInjectExtractPropagationModes(
    const std::vector<PropagationMode>& propagation_modes,
    std::vector<PropagationMode>& inject_propagation_modes,
    std::vector<PropagationMode>& extract_propagation_modes);
}  // namespace lightstep

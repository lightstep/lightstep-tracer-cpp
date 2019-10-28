#include "tracer/propagation_options.h"

#include <set>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// UniqifyPropagationModes
//--------------------------------------------------------------------------------------------------
static void UniqifyPropagationModes(
    std::vector<PropagationMode>& propagation_modes) {
  // Note: The order of propagation modes is significant: Extracts are tried in
  // the order given until one of them succeeds. Hence, we use uniqifying logic
  // that preserves the ordering.
  std::vector<PropagationMode> result;
  result.reserve(propagation_modes.size());
  std::set<PropagationMode> values;
  for (auto propagation_mode : propagation_modes) {
    if (values.insert(propagation_mode).second) {
      result.push_back(propagation_mode);
    }
  }
  propagation_modes.swap(result);
}

//--------------------------------------------------------------------------------------------------
// SetInjectExtractPropagationModes
//--------------------------------------------------------------------------------------------------
void SetInjectExtractPropagationModes(
    const std::vector<PropagationMode>& propagation_modes,
    std::vector<PropagationMode>& inject_propagation_modes,
    std::vector<PropagationMode>& extract_propagation_modes) {
  inject_propagation_modes.clear();
  inject_propagation_modes.reserve(propagation_modes.size());

  extract_propagation_modes.clear();
  extract_propagation_modes.reserve(propagation_modes.size());

  for (auto propagation_mode : propagation_modes) {
    inject_propagation_modes.push_back(propagation_mode);
    extract_propagation_modes.push_back(propagation_mode);
    if (propagation_mode == PropagationMode::envoy) {
      // Envoy always falls back to the traditional LightStep extraction if the
      // single-header propagation mode fails.
      extract_propagation_modes.push_back(PropagationMode::lightstep);
    }
  }

  UniqifyPropagationModes(inject_propagation_modes);
  UniqifyPropagationModes(extract_propagation_modes);
}
}  // namespace lightstep

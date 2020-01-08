#include "tracer/propagation/propagation_options.h"

#include <set>

#include "tracer/propagation/b3_propagator.h"
#include "tracer/propagation/baggage_propagator.h"
#include "tracer/propagation/envoy_propagator.h"
#include "tracer/propagation/lightstep_propagator.h"
#include "tracer/propagation/trace_context_propagator.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// UniqifyPropagationModes
//--------------------------------------------------------------------------------------------------
static void UniqifyPropagationModes(
    std::vector<PropagationMode>& propagation_modes) {
  // Note: The order of propagation modes is significant: Extracts are tried in
  // the order given until one of them succeeds. Hence, we use uniqifying logic
  // that preserves ordering.
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

//--------------------------------------------------------------------------------------------------
// MakePropagators
//--------------------------------------------------------------------------------------------------
static std::vector<std::unique_ptr<Propagator>> MakePropagators(
    const std::vector<PropagationMode>& propagation_modes) {
  std::vector<std::unique_ptr<Propagator>> result;
  result.reserve(propagation_modes.size());
  for (auto propagation_mode : propagation_modes) {
    switch (propagation_mode) {
      case PropagationMode::lightstep:
        result.emplace_back(MakeLightStepPropagator());
        break;
      case PropagationMode::b3:
        result.emplace_back(MakeB3Propagator());
        break;
      case PropagationMode::envoy:
        result.emplace_back(new EnvoyPropagator{});
        break;
      case PropagationMode::trace_context:
        result.emplace_back(new TraceContextPropagator{});
        break;
    }
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// MakePropagationOptions
//--------------------------------------------------------------------------------------------------
PropagationOptions MakePropagationOptions(
    const LightStepTracerOptions& options) {
  std::vector<PropagationMode> propagation_modes;
  PropagationOptions result;
  if (!options.propagation_modes.empty()) {
    propagation_modes = options.propagation_modes;
  } else if (options.use_single_key_propagation) {
    propagation_modes = {PropagationMode::envoy};
  } else {
    propagation_modes = {PropagationMode::lightstep};
  }

  std::vector<PropagationMode> inject_propagation_modes;
  std::vector<PropagationMode> extract_propagation_modes;

  SetInjectExtractPropagationModes(propagation_modes, inject_propagation_modes,
                                   extract_propagation_modes);
  result.inject_propagators = MakePropagators(inject_propagation_modes);
  result.extract_propagators = MakePropagators(extract_propagation_modes);
  if (!(propagation_modes.size() == 1 &&
        propagation_modes[0] == PropagationMode::envoy)) {
    result.inject_propagators.emplace_back(
        new BaggagePropagator{PrefixBaggage});
  }
  return result;
}
}  // namespace lightstep

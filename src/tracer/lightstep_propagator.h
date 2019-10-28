#pragma once

#include <memory>

#include "tracer/propagator.h"

#include <opentracing/string_view.h>

namespace lightstep {
extern const opentracing::string_view PrefixBaggage;

class LightStepPropagator {
 public:
  opentracing::string_view trace_id_key() const noexcept;

  opentracing::string_view span_id_key() const noexcept;

  opentracing::string_view sampled_key() const noexcept;

  opentracing::string_view baggage_prefix() const noexcept;

  bool supports_baggage() const noexcept { return true; }
};

std::unique_ptr<Propagator> MakeLightStepPropagator();
}  // namespace lightstep

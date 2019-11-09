#pragma once

#include "tracer/lightstep_span_context.h"

#include <opentracing/tracer.h>

#include <stdexcept>
#include <vector>

namespace lightstep {
inline bool AreSpanContextsEquivalent(
    const opentracing::SpanContext& context1,
    const opentracing::SpanContext& context2) {
  return dynamic_cast<const lightstep::LightStepSpanContext&>(context1) ==
         dynamic_cast<const lightstep::LightStepSpanContext&>(context2);
}

template <class Carrier>
void VerifyInjectExtract(const opentracing::Tracer& tracer,
                         const opentracing::SpanContext& span_context,
                         Carrier& carrier) {
  auto rcode = tracer.Inject(span_context, carrier);
  if (!rcode) {
    throw std::runtime_error{"failed to inject span context: " +
                             rcode.error().message()};
  }
  auto span_context_maybe = tracer.Extract(carrier);
  if (!span_context_maybe) {
    throw std::runtime_error{"failed to extract span context: " +
                             span_context_maybe.error().message()};
  }
  if (*span_context_maybe == nullptr) {
    throw std::runtime_error{"no span context extracted"};
  }

  if (!AreSpanContextsEquivalent(span_context, **span_context_maybe)) {
    throw std::runtime_error{"span contexts not equal"};
  }
}

std::vector<std::unique_ptr<opentracing::SpanContext>> MakeTestSpanContexts(
    bool use_128bit_trace_ids = false);
}  // namespace lightstep

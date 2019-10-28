#include "lightstep_propagator.h"

#include "multiheader_propagator.h"

#define PREFIX_TRACER_STATE "ot-tracer-"
// Note: these constants are a convention of the OpenTracing basictracers.

const opentracing::string_view FieldNameTraceID = PREFIX_TRACER_STATE "traceid";
const opentracing::string_view FieldNameSpanID = PREFIX_TRACER_STATE "spanid";
const opentracing::string_view FieldNameSampled = PREFIX_TRACER_STATE "sampled";
#undef PREFIX_TRACER_STATE

namespace lightstep {
const opentracing::string_view PrefixBaggage = "ot-baggage-";

//--------------------------------------------------------------------------------------------------
// trace_id_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view LightStepPropagator::trace_id_key() const noexcept {
  return FieldNameTraceID;
}

//--------------------------------------------------------------------------------------------------
// span_id_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view LightStepPropagator::span_id_key() const noexcept {
  return FieldNameSpanID;
}

//--------------------------------------------------------------------------------------------------
// sampled_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view LightStepPropagator::sampled_key() const noexcept {
  return FieldNameSampled;
}

//--------------------------------------------------------------------------------------------------
// baggage_prefix
//--------------------------------------------------------------------------------------------------
opentracing::string_view LightStepPropagator::baggage_prefix() const noexcept {
  return PrefixBaggage;
}

//--------------------------------------------------------------------------------------------------
// MakeLightStepPropagator
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Propagator> MakeLightStepPropagator() {
  return std::unique_ptr<Propagator>{
      new MultiheaderPropagator{FieldNameTraceID, FieldNameSpanID,
                                FieldNameSampled, PrefixBaggage, false}};
}
}  // namespace lightstep

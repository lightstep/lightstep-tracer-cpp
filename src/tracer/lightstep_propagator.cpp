#include "lightstep_propagator.h"

#include "tracer/propagation/multiheader_propagator.h"

#define PREFIX_TRACER_STATE "ot-tracer-"
// Note: these constants are a convention of the OpenTracing basictracers.

const opentracing::string_view FieldNameTraceID = PREFIX_TRACER_STATE "traceid";
const opentracing::string_view FieldNameSpanID = PREFIX_TRACER_STATE "spanid";
const opentracing::string_view FieldNameSampled = PREFIX_TRACER_STATE "sampled";
#undef PREFIX_TRACER_STATE

namespace lightstep {
const opentracing::string_view PrefixBaggage = "ot-baggage-";

//--------------------------------------------------------------------------------------------------
// MakeLightStepPropagator
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Propagator> MakeLightStepPropagator() {
  return std::unique_ptr<Propagator>{
      new MultiheaderPropagator{FieldNameTraceID, FieldNameSpanID,
                                FieldNameSampled, PrefixBaggage, false}};
}
}  // namespace lightstep

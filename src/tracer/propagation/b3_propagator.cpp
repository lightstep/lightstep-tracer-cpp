#include "tracer/propagation/b3_propagator.h"

#include "tracer/propagation/lightstep_propagator.h"
#include "tracer/propagation/multiheader_propagator.h"

#define PREFIX_TRACER_STATE "X-B3-"
const opentracing::string_view FieldNameTraceID = PREFIX_TRACER_STATE "TraceId";
const opentracing::string_view FieldNameSpanID = PREFIX_TRACER_STATE "SpanId";
const opentracing::string_view FieldNameSampled = PREFIX_TRACER_STATE "Sampled";
#undef PREFIX_TRACER_STATE

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeB3Propagator
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Propagator> MakeB3Propagator() {
  return std::unique_ptr<Propagator>{
      new MultiheaderPropagator{FieldNameTraceID, FieldNameSpanID,
                                FieldNameSampled, PrefixBaggage, true}};
}
}  // namespace lightstep

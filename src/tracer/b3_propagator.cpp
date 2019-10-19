#include "b3_propagator.h"

#define PREFIX_TRACER_STATE "X-B3-"
const opentracing::string_view FieldNameTraceID = PREFIX_TRACER_STATE "TraceId";
const opentracing::string_view FieldNameSpanID = PREFIX_TRACER_STATE "SpanId";
const opentracing::string_view FieldNameSampled = PREFIX_TRACER_STATE "Sampled";
#undef PREFIX_TRACER_STATE

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// trace_id_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view B3Propagator::trace_id_key() const noexcept {
  return FieldNameTraceID;
}

//--------------------------------------------------------------------------------------------------
// span_id_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view B3Propagator::span_id_key() const noexcept {
  return FieldNameSpanID;
}

//--------------------------------------------------------------------------------------------------
// sampled_key
//--------------------------------------------------------------------------------------------------
opentracing::string_view B3Propagator::sampled_key() const noexcept {
  return FieldNameSampled;
}
}  // namespace lightstep

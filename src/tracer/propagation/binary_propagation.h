#pragma once

#include <iosfwd>

#include "tracer/propagation/propagator.h"

#include <google/protobuf/map.h>
#include <opentracing/propagation.h>

namespace lightstep {
template <class BaggageMap>
opentracing::expected<void> InjectSpanContext(std::ostream& carrier,
                                              uint64_t trace_id,
                                              uint64_t span_id, bool sampled,
                                              const BaggageMap& baggage);

opentracing::expected<bool> ExtractSpanContext(std::istream& carrier,
                                               uint64_t& trace_id,
                                               uint64_t& span_id, bool& sampled,
                                               BaggageProtobufMap& baggage);

}  // namespace lightstep

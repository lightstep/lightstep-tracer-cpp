#pragma once

#include "tracer/baggage_flat_map.h"
#include "tracer/propagation/trace_context.h"

#include <google/protobuf/map.h>

#include <opentracing/propagation.h>

namespace lightstep {
using BaggageProtobufMap = google::protobuf::Map<std::string, std::string>;

class Propagator {
 public:
  virtual ~Propagator() noexcept = default;

  virtual opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context, opentracing::string_view trace_state,
      const BaggageProtobufMap& baggage) const = 0;

  virtual opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier,
      const TraceContext& trace_context, opentracing::string_view trace_state,
      const BaggageFlatMap& baggage) const = 0;

  virtual opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      TraceContext& trace_context, std::string& trace_state,
      BaggageProtobufMap& baggage) const = 0;
};
}  // namespace lightstep

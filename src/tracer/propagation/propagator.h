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
      uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
      bool& sampled, BaggageProtobufMap& baggage) const = 0;

  virtual opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      TraceContext& trace_context, std::string& trace_state,
      BaggageProtobufMap& baggage) const {
    (void)trace_state;
    bool sampled;
    auto result = ExtractSpanContext(
        carrier, case_sensitive, trace_context.trace_id_high,
        trace_context.trace_id_low, trace_context.parent_id, sampled, baggage);
    if (!result || !*result) {
      return result;
    }
    trace_context.trace_flags = SetTraceFlag<SampledFlagMask>(0, sampled);
    if (trace_context.trace_id_high == 0 && trace_context.trace_id_low == 0) {
      return opentracing::make_unexpected(
          opentracing::invalid_span_context_error);
    }
    return true;
  }
};
}  // namespace lightstep

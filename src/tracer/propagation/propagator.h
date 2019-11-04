#pragma once

#include "tracer/baggage_flat_map.h"

#include <google/protobuf/map.h>

#include <opentracing/propagation.h>

namespace lightstep {
using BaggageProtobufMap = google::protobuf::Map<std::string, std::string>;

class Propagator {
 public:
  virtual ~Propagator() noexcept = default;

  virtual opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageProtobufMap& baggage) const = 0;

  virtual opentracing::expected<void> InjectSpanContext(
      const opentracing::TextMapWriter& carrier, uint64_t trace_id_high,
      uint64_t trace_id_low, uint64_t span_id, bool sampled,
      const BaggageFlatMap& baggage) const = 0;

  virtual opentracing::expected<bool> ExtractSpanContext(
      const opentracing::TextMapReader& carrier, bool case_sensitive,
      uint64_t& trace_id_high, uint64_t& trace_id_low, uint64_t& span_id,
      bool& sampled, BaggageProtobufMap& baggage) const = 0;
};
}  // namespace lightstep

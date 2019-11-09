#include "test/tracer/propagation/utility.h"

#include "tracer/legacy/legacy_immutable_span_context.h"

namespace lightstep {
//------------------------------------------------------------------------------
// MakeTestSpanContexts
//------------------------------------------------------------------------------
// A vector of different types of span contexts to test against.
std::vector<std::unique_ptr<opentracing::SpanContext>> MakeTestSpanContexts() {
  std::vector<std::unique_ptr<opentracing::SpanContext>> result;
  // most basic span context
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new LegacyImmutableSpanContext{
          123, 456, true, std::unordered_map<std::string, std::string>{}}});

  // span context with single baggage item
  result.push_back(std::unique_ptr<opentracing::SpanContext>{
      new LegacyImmutableSpanContext{123, 456, true, {{"abc", "123"}}}});

  // span context with multiple baggage items
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new LegacyImmutableSpanContext{
          123, 456, true, {{"abc", "123"}, {"xyz", "qrz"}}}});

  // unsampled span context
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new LegacyImmutableSpanContext{
          123, 456, false, std::unordered_map<std::string, std::string>{}}});

  return result;
}
}  // namespace lightstep

#include "test/tracer/propagation/utility.h"

#include <random>
#include <tuple>

#include "tracer/immutable_span_context.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GenerateTraceID
//--------------------------------------------------------------------------------------------------
static std::tuple<uint64_t, uint64_t> GenerateTraceID(bool use_128bit_ids) {
  static thread_local std::mt19937_64 random_number_generator{0};
  if (use_128bit_ids) {
    return {random_number_generator(), random_number_generator()};
  }
  return {0, random_number_generator()};
}

//------------------------------------------------------------------------------
// MakeTestSpanContexts
//------------------------------------------------------------------------------
// A vector of different types of span contexts to test against.
std::vector<std::unique_ptr<opentracing::SpanContext>> MakeTestSpanContexts(
    bool use_128bit_trace_ids) {
  uint64_t trace_id_low, trace_id_high;
  std::vector<std::unique_ptr<opentracing::SpanContext>> result;
  // most basic span context
  std::tie(trace_id_high, trace_id_low) = GenerateTraceID(use_128bit_trace_ids);
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new ImmutableSpanContext{
          trace_id_high, trace_id_low, 456, true,
          std::unordered_map<std::string, std::string>{}}});

  // span context with single baggage item
  std::tie(trace_id_high, trace_id_low) = GenerateTraceID(use_128bit_trace_ids);
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new ImmutableSpanContext{
          trace_id_high, trace_id_low, 456, true, {{"abc", "123"}}}});

  // span context with multiple baggage items
  std::tie(trace_id_high, trace_id_low) = GenerateTraceID(use_128bit_trace_ids);
  result.push_back(std::unique_ptr<opentracing::SpanContext>{
      new ImmutableSpanContext{trace_id_high,
                               trace_id_low,
                               456,
                               true,
                               {{"abc", "123"}, {"xyz", "qrz"}}}});

  // unsampled span context
  std::tie(trace_id_high, trace_id_low) = GenerateTraceID(use_128bit_trace_ids);
  result.push_back(
      std::unique_ptr<opentracing::SpanContext>{new ImmutableSpanContext{
          trace_id_high, trace_id_low, 456, false,
          std::unordered_map<std::string, std::string>{}}});

  return result;
}
}  // namespace lightstep

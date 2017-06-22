#include <lightstep/binary_protocol.h>
#include <lightstep/tracer.h>
using namespace opentracing;

namespace lightstep {
//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
Expected<std::unique_ptr<SpanContext>> LightStepBinaryReader::Extract(
    const Tracer& tracer) const try {
  auto lightstep_tracer = dynamic_cast<const LightStepTracer*>(&tracer);
  if (!lightstep_tracer) {
    return make_unexpected(invalid_carrier_error);
  }
  if (!carrier_) return {};
  auto& basic = carrier_->basic_ctx();
  std::unordered_map<std::string, std::string> baggage;
  for (const auto& entry : basic.baggage_items()) {
    baggage.emplace(entry.first, entry.second);
  }
  return lightstep_tracer->MakeSpanContext(basic.trace_id(), basic.span_id(),
                                           std::move(baggage));
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
Expected<void> LightStepBinaryWriter::Inject(
    const opentracing::Tracer& tracer, const opentracing::SpanContext& sc) const
    try {
  auto lightstep_tracer = dynamic_cast<const LightStepTracer*>(&tracer);
  if (!lightstep_tracer) {
    return make_unexpected(invalid_carrier_error);
  }
  auto trace_span_ids_maybe = lightstep_tracer->GetTraceSpanIds(sc);
  if (!trace_span_ids_maybe)
    return make_unexpected(trace_span_ids_maybe.error());
  auto& trace_span_ids = *trace_span_ids_maybe;
  carrier_.Clear();
  auto basic = carrier_.mutable_basic_ctx();
  basic->set_trace_id(trace_span_ids[0]);
  basic->set_span_id(trace_span_ids[1]);
  basic->set_sampled(true);

  auto baggage = basic->mutable_baggage_items();

  sc.ForeachBaggageItem(
      [baggage](const std::string& key, const std::string& value) {
        (*baggage)[key] = value;
        return true;
      });
  return {};
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}
}  // namespace lightstep

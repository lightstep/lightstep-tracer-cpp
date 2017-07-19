#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <lightstep/version.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <opentracing/version.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include "lightstep_span_context.h"
#include "lightstep_tracer_impl.h"
#include "recorder.h"
#include "utility.h"
using namespace opentracing;

namespace lightstep {
const char* const component_name_key = "lightstep.component_name";

const std::pair<const char*, opentracing::Value> default_tags[] = {
    {"lightstep.tracer_platform", "c++"},
    {"lightstep.tracer_platform_version", __cplusplus},
    {"lightstep.tracer_version", LIGHTSTEP_VERSION},
    {"lightstep.opentracing_version", OPENTRACING_VERSION}};

//------------------------------------------------------------------------------
// GetTraceSpanIds
//------------------------------------------------------------------------------
expected<std::array<uint64_t, 2>> LightStepTracer::GetTraceSpanIds(
    const SpanContext& sc) const noexcept {
  auto lightstep_span_context = dynamic_cast<const LightStepSpanContext*>(&sc);
  if (lightstep_span_context == nullptr) {
    return make_unexpected(invalid_span_context_error);
  }
  std::array<uint64_t, 2> result = {
      {lightstep_span_context->trace_id(), lightstep_span_context->span_id()}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
expected<std::unique_ptr<SpanContext>> LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<SpanContext> result(
      new LightStepSpanContext(trace_id, span_id, std::move(baggage)));
  return result;
} catch (const std::bad_alloc&) {
  return make_unexpected(make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    const LightStepTracerOptions& options) {
  auto options_new = options;

  // Copy over default tags.
  for (const auto& tag : default_tags) {
    options_new.tags[tag.first] = tag.second;
  }

  // Set the component name if provided or default it to the program name.
  if (!options.component_name.empty()) {
    options_new.tags[component_name_key] = options.component_name;
  } else {
    options_new.tags.emplace(component_name_key, GetProgramName());
  }

  auto recorder = make_lightstep_recorder(options);
  if (!recorder) {
    return nullptr;
  }
  return std::shared_ptr<opentracing::Tracer>(
      new (std::nothrow) LightStepTracerImpl(std::move(recorder)));
}
}  // namespace lightstep

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
#include "buffered_recorder.h"
#include "grpc_transporter.h"
#include "lightstep_span_context.h"
#include "lightstep_tracer_impl.h"
#include "logger.h"
#include "utility.h"

namespace lightstep {
const opentracing::string_view component_name_key = "lightstep.component_name";

//------------------------------------------------------------------------------
// GetDefaultTags
//------------------------------------------------------------------------------
static const std::vector<
    std::pair<opentracing::string_view, opentracing::Value>>&
GetDefaultTags() {
  static const std::vector<
      std::pair<opentracing::string_view, opentracing::Value>>
      default_tags = {{"lightstep.tracer_platform", "c++"},
                      {"lightstep.tracer_platform_version", __cplusplus},
                      {"lightstep.tracer_version", LIGHTSTEP_VERSION},
                      {"lightstep.opentracing_version", OPENTRACING_VERSION}};
  return default_tags;
}

//------------------------------------------------------------------------------
// GetTraceSpanIds
//------------------------------------------------------------------------------
opentracing::expected<std::array<uint64_t, 2>> LightStepTracer::GetTraceSpanIds(
    const opentracing::SpanContext& span_context) const noexcept {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  std::array<uint64_t, 2> result = {
      {lightstep_span_context->trace_id(), lightstep_span_context->span_id()}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<opentracing::SpanContext> result{
      new LightStepSpanContext{trace_id, span_id, std::move(baggage)}};
  return result;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> MakeLightStepTracer(
    LightStepTracerOptions options) noexcept try {
  // Create and configure the logger.
  auto& logger = GetLogger();
  if (options.verbose) {
    logger.set_level(spdlog::level::info);
  } else {
    logger.set_level(spdlog::level::err);
  }

  // Validate `options`.
  if (options.access_token.empty()) {
    logger.error("Must provide an access_token!");
    return nullptr;
  }

  // Copy over default tags.
  for (const auto& tag : GetDefaultTags()) {
    options.tags[tag.first] = tag.second;
  }

  // Set the component name if not provided to the program name.
  if (options.component_name.empty()) {
    options.tags.emplace(component_name_key, GetProgramName());
  } else {
    options.tags.emplace(component_name_key, options.component_name);
  }

  auto transporter = std::unique_ptr<Transporter>{new GrpcTransporter{options}};
  auto recorder = std::unique_ptr<Recorder>{
      new BufferedRecorder{std::move(options), std::move(transporter)}};
  return std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::move(recorder)}};
} catch (const spdlog::spdlog_ex& e) {
  std::cerr << "Log init failed: " << e.what() << "\n";
  return nullptr;
} catch (const std::exception& e) {
  GetLogger().error("Failed to construct LightStep Tracer: {}", e.what());
  return nullptr;
}
}  // namespace lightstep

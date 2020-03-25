#include "lightstep/tracer.h"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "common/logger.h"
#include "common/platform/utility.h"
#include "common/utility.h"
#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep/version.h"
#include "recorder/auto_recorder.h"
#include "recorder/grpc_transporter.h"
#include "recorder/legacy_manual_recorder.h"
#include "recorder/manual_recorder.h"
#include "recorder/stream_recorder.h"
#include "tracer/immutable_span_context.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "opentracing/string_view.h"
#include "opentracing/value.h"
#include "opentracing/version.h"

namespace lightstep {
const opentracing::string_view component_name_key = "lightstep.component_name";
const opentracing::string_view collector_service_full_name =
    "lightstep.collector.CollectorService";
const opentracing::string_view collector_method_name = "Report";

extern const unsigned char default_ssl_roots_pem[];
extern const int default_ssl_roots_pem_size;

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
// CollectorServiceFullName
//------------------------------------------------------------------------------
const std::string& CollectorServiceFullName() {
  static std::string name = collector_service_full_name;
  return name;
}

//------------------------------------------------------------------------------
// CollectorMethodName
//------------------------------------------------------------------------------
const std::string& CollectorMethodName() {
  static std::string name = collector_method_name;
  return name;
}

//------------------------------------------------------------------------------
// GetTraceSpanIdsSampled
//------------------------------------------------------------------------------
opentracing::expected<std::array<uint64_t, 3>>
LightStepTracer::GetTraceSpanIdsSampled(
    const opentracing::SpanContext& span_context) const noexcept {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  std::array<uint64_t, 3> result = {
      {lightstep_span_context->trace_id_low(),
       lightstep_span_context->span_id(),
       static_cast<uint64_t>(lightstep_span_context->sampled())}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id, bool sampled,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<opentracing::SpanContext> result{
      new ImmutableSpanContext{0, trace_id, span_id, sampled, baggage}};
  return result;
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeThreadedTracer
//------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeThreadedTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  std::unique_ptr<SyncTransporter> transporter;
  if (options.transporter != nullptr) {
    transporter = std::unique_ptr<SyncTransporter>{
        dynamic_cast<SyncTransporter*>(options.transporter.get())};
    if (transporter == nullptr) {
      logger->Error(
          "`options.transporter` must be derived from SyncTransporter");
      return nullptr;
    }
    options.transporter.release();
  } else {
    transporter = MakeGrpcTransporter(*logger, options);
  }
  auto propagation_options = MakePropagationOptions(options);
  auto recorder = std::unique_ptr<Recorder>{
      new AutoRecorder{*logger, std::move(options), std::move(transporter)}};
  return std::shared_ptr<LightStepTracer>{new LegacyTracerImpl{
      std::move(logger), std::move(propagation_options), std::move(recorder)}};
}

//--------------------------------------------------------------------------------------------------
// MakeStreamTracer
//--------------------------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeStreamTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  auto propagation_options = MakePropagationOptions(options);
  auto recorder = MakeStreamRecorder(*logger, std::move(options));
  return std::shared_ptr<LightStepTracer>{new TracerImpl{
      std::move(logger), std::move(propagation_options), std::move(recorder)}};
}

//------------------------------------------------------------------------------
// MakeLegacySingleThreadedTracer
//------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeLegacySingleThreadedTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  std::unique_ptr<LegacyAsyncTransporter> transporter;
  if (options.transporter != nullptr) {
    transporter = std::unique_ptr<LegacyAsyncTransporter>{
        dynamic_cast<LegacyAsyncTransporter*>(options.transporter.get())};
    if (transporter == nullptr) {
      logger->Error(
          "`options.transporter` must be derived from AsyncTransporter "
          "LegacyAsyncTransporter");
      return nullptr;
    }
    options.transporter.release();
  } else {
    logger->Error(
        "`options.transporter` must be set if `options.use_thread` is false");
    return nullptr;
  }
  auto propagation_options = MakePropagationOptions(options);
  auto recorder = std::unique_ptr<Recorder>{new LegacyManualRecorder{
      *logger, std::move(options), std::move(transporter)}};
  return std::shared_ptr<LightStepTracer>{new LegacyTracerImpl{
      std::move(logger), std::move(propagation_options), std::move(recorder)}};
}

//------------------------------------------------------------------------------
// MakeSingleThreadedTracer
//------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeSingleThreadedTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  std::unique_ptr<AsyncTransporter> transporter;
  if (options.transporter != nullptr) {
    transporter = std::unique_ptr<AsyncTransporter>{
        dynamic_cast<AsyncTransporter*>(options.transporter.get())};
    if (transporter == nullptr) {
      return MakeLegacySingleThreadedTracer(logger, std::move(options));
    }
    options.transporter.release();
  } else {
    logger->Error(
        "`options.transporter` must be set if `options.use_thread` is false");
    return nullptr;
  }
  auto propagation_options = MakePropagationOptions(options);
  auto recorder = std::unique_ptr<Recorder>{
      new ManualRecorder{*logger, std::move(options), std::move(transporter)}};
  return std::shared_ptr<LightStepTracer>{new TracerImpl{
      std::move(logger), std::move(propagation_options), std::move(recorder)}};
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<LightStepTracer> MakeLightStepTracer(
    LightStepTracerOptions&& options) noexcept try {
  // Create and configure the logger.
  auto logger = std::make_shared<Logger>(std::move(options.logger_sink));
  try {
    if (options.verbose) {
      logger->set_level(LogLevel::info);
    } else {
      logger->set_level(LogLevel::error);
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

    if (options.ssl_root_certificates.empty()) {
      // If default ssl root certificates were embedded in the library, then
      // default_ssl_roots_pem will be set to them; otherwise, it will be the
      // empty string and GRPC will attempt to apply its defaults.
      options.ssl_root_certificates =
          std::string{reinterpret_cast<const char*>(default_ssl_roots_pem),
                      static_cast<size_t>(default_ssl_roots_pem_size)};
    }

    if (!options.use_thread) {
      return MakeSingleThreadedTracer(logger, std::move(options));
    }
    if (options.use_stream_recorder) {
      if (!options.collector_plaintext) {
        logger->Error("Encrypted streaming not supported yet");
        return nullptr;
      }
      if (!options.use_thread) {
        logger->Error("Stream recorder only supports threaded mode");
        return nullptr;
      }
      if (options.transporter != nullptr) {
        logger->Error("Stream recorder doesn't support custom transports");
        return nullptr;
      }
      return MakeStreamTracer(logger, std::move(options));
    }
    return MakeThreadedTracer(logger, std::move(options));
  } catch (const std::exception& e) {
    logger->Error("Failed to construct LightStep Tracer: ", e.what());
    return nullptr;
  }
} catch (const std::exception& e) {
  // Use fprintf to print the error because std::cerr can throw the user
  // configures by calling std::cerr::exceptions.
  std::fprintf(stderr, "Failed to initialize logger: %s\n", e.what());
  return nullptr;
}
}  // namespace lightstep

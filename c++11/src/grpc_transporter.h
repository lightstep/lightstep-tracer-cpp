#pragma once

#include <lightstep/spdlog/logger.h>
#include <lightstep/tracer.h>

namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    spdlog::logger& logger, const LightStepTracerOptions& options);
}  // namespace lightstep

#pragma once

#include "common/logger.h"
#include "lightstep/tracer.h"

namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    Logger& logger, const LightStepTracerOptions& options);
}  // namespace lightstep

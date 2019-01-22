#pragma once

#include "lightstep/tracer.h"
#include "common/logger.h"
#include "recorder/recorder.h"

namespace lightstep {
std::unique_ptr<Recorder> MakeStreamRecorder(
    Logger& logger, LightStepTracerOptions&& tracer_options);
}  // namespace lightstep

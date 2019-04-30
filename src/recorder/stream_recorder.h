#pragma once

#include "common/logger.h"
#include "lightstep/tracer.h"
#include "recorder/recorder.h"

namespace lightstep {
/**
 * Constructs a StreamRecorder.
 * @param logger supplies the place to write logs.
 * @params tracer_options supplies the user-provided configuration options to
 * apply.
 * @return a StreamRecorder
 */
std::unique_ptr<Recorder> MakeStreamRecorder(
    Logger& logger, LightStepTracerOptions&& tracer_options);
}  // namespace lightstep

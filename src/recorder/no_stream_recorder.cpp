#include "recorder/stream_recorder.h"

#include <stdexcept>

namespace lightstep {
std::unique_ptr<Recorder> MakeStreamRecorder(
    Logger& /*logger*/, LightStepTracerOptions&& /*tracer_options*/) {
  throw std::runtime_error{
      "LightStep was not built with stream recorder support, so a different "
      "recorder must be selected."};
}
}  // namespace lightstep

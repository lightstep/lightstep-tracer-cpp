#pragma once

#include "common/serialization_chain.h"
#include "common/noncopyable.h"
#include "recorder/fork_aware_recorder.h"
#include "recorder/stream_recorder/stream_recorder_options.h"

namespace lightstep {
class StreamRecorder2 : public ForkAwareRecorder, private Noncopyable {
 public:
  StreamRecorder2(
      Logger& logger, LightStepTracerOptions&& tracer_options,
      StreamRecorderOptions&& recorder_options = StreamRecorderOptions{});

   // Recorder
  void RecordSpan(std::unique_ptr<SerializationChain>&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;
 private:
};
} // namespace lightstep

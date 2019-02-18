#pragma once

#include "common/chunk_circular_buffer.h"
#include "common/noncopyable.h"
#include "recorder/stream_recorder/satellite_endpoint_manager.h"

namespace lightstep {
class SatelliteConnectionManager : private Noncopyable {
 public:
  SatelliteConnectionManager(Logger& logger, EventBase& event_base,
                             const LightStepTracerOptions& tracer_options,
                             const StreamRecorderOptions& recorder_options,
                             ChunkCircularBuffer& span_buffer);

 private:
  SatelliteEndpointManager endpoint_manager_;
  ChunkCircularBuffer& span_buffer_;

  void OnEndpointManagerReady() noexcept;
};
}  // namespace lightstep

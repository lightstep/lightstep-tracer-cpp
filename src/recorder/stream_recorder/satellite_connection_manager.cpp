#include "recorder/stream_recorder/satellite_connection_manager.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteConnectionManager::SatelliteConnectionManager(
    Logger& logger, EventBase& event_base,
    const LightStepTracerOptions& tracer_options,
    const StreamRecorderOptions& recorder_options,
    ChunkCircularBuffer& span_buffer)
    : endpoint_manager_{logger, event_base, tracer_options, recorder_options,
                        [this] { this->OnEndpointManagerReady(); }},
      span_buffer_{span_buffer} {}

//--------------------------------------------------------------------------------------------------
// OnEndpointManagerReady
//--------------------------------------------------------------------------------------------------
void SatelliteConnectionManager::OnEndpointManagerReady() noexcept {}
}  // namespace lightstep

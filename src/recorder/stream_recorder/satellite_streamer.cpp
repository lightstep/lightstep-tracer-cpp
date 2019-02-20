#include "recorder/stream_recorder/satellite_streamer.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteStreamer::SatelliteStreamer(
    Logger& logger, EventBase& event_base,
    const LightStepTracerOptions& tracer_options,
    const StreamRecorderOptions& recorder_options,
    ChunkCircularBuffer& span_buffer)
    : logger_{logger},
      event_base_{event_base},
      recorder_options_{recorder_options},
      endpoint_manager_{logger, event_base, tracer_options, recorder_options,
                        [this] { this->OnEndpointManagerReady(); }},
      span_buffer_{span_buffer} {
  connections_.reserve(recorder_options.num_satellite_connections);
  writable_connections_.reserve(recorder_options.num_satellite_connections);
  for (int i = 0; i < recorder_options.num_satellite_connections; ++i) {
    connections_.emplace_back(new SatelliteConnection{*this});
  }
}

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::Flush() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnConnectionWritable
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::OnConnectionWritable(
    SatelliteConnection* /*connection*/) noexcept {}

//--------------------------------------------------------------------------------------------------
// OnEndpointManagerReady
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::OnEndpointManagerReady() noexcept {
  for (auto& connection : connections_) {
    connection->Start();
  }
}
}  // namespace lightstep

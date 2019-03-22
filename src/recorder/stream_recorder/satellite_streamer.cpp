#include "recorder/stream_recorder/satellite_streamer.h"

#include <algorithm>
#include <cassert>

#include "common/random.h"
#include "recorder/stream_recorder/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SatelliteStreamer::SatelliteStreamer(
    Logger& logger, EventBase& event_base,
    const LightStepTracerOptions& tracer_options,
    const StreamRecorderOptions& recorder_options,
    StreamRecorderMetrics& metrics, ChunkCircularBuffer& span_buffer)
    : logger_{logger},
      event_base_{event_base},
      tracer_options_{tracer_options},
      recorder_options_{recorder_options},
      header_common_fragment_{
          WriteStreamHeaderCommonFragment(tracer_options, GenerateId())},
      endpoint_manager_{logger, event_base, tracer_options, recorder_options,
                        [this] { this->OnEndpointManagerReady(); }},
      metrics_{metrics},
      span_buffer_{span_buffer},
      span_stream_{span_buffer, recorder_options_.num_satellite_connections},
      connection_traverser_{recorder_options_.num_satellite_connections} {
  connections_.reserve(recorder_options.num_satellite_connections);
  for (int i = 0; i < recorder_options.num_satellite_connections; ++i) {
    connections_.emplace_back(new SatelliteConnection{*this});
  }
  endpoint_manager_.Start();
}

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::Flush() noexcept {
  if (span_buffer_.empty()) {
    return;
  }
  connection_traverser_.ForEachIndex([this](int index) {
    auto& connection = connections_[index];
    if (!connection->ready()) {
      return true;
    }
    return !connection->Flush();
  });
}

//--------------------------------------------------------------------------------------------------
// OnEndpointManagerReady
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::OnEndpointManagerReady() noexcept {
  for (auto& connection : connections_) {
    connection->Start();
  }
}
}  // namespace lightstep

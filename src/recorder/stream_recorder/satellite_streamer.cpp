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
      recorder_options_{recorder_options},
      header_common_fragment_{
          WriteStreamHeaderCommonFragment(tracer_options, GenerateId())},
      endpoint_manager_{logger, event_base, tracer_options, recorder_options,
                        [this] { this->OnEndpointManagerReady(); }},
      metrics_{metrics},
      span_buffer_{span_buffer} {
  connections_.reserve(recorder_options.num_satellite_connections);
  writable_connections_.reserve(recorder_options.num_satellite_connections);
  for (int i = 0; i < recorder_options.num_satellite_connections; ++i) {
    connections_.emplace_back(new SatelliteConnection{*this});
  }
  endpoint_manager_.Start();
}

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::Flush() noexcept {
  // Stub that will be replaced by code that sends spans to satellites.
  if (writable_connections_.empty()) {
    return;
  }
  while (true) {
    span_buffer_.Allot();
    span_buffer_.Consume(span_buffer_.num_bytes_allotted());
    if (span_buffer_.empty()) {
      break;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// OnConnectionWritable
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::OnConnectionWritable(
    SatelliteConnection* connection) noexcept {
  assert(std::find(writable_connections_.begin(), writable_connections_.end(),
                   connection) == writable_connections_.end());
  writable_connections_.emplace_back(connection);
}

//--------------------------------------------------------------------------------------------------
// OnConnectionNonwritable
//--------------------------------------------------------------------------------------------------
void SatelliteStreamer::OnConnectionNonwritable(
    SatelliteConnection* connection) noexcept {
  writable_connections_.erase(
      std::remove(writable_connections_.begin(), writable_connections_.end(),
                  connection),
      writable_connections_.end());
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

#pragma once

#include <memory>
#include <vector>

#include "common/chunk_circular_buffer.h"
#include "common/noncopyable.h"
#include "recorder/stream_recorder/satellite_connection.h"
#include "recorder/stream_recorder/satellite_endpoint_manager.h"

namespace lightstep {
class SatelliteStreamer : private Noncopyable {
 public:
  SatelliteStreamer(Logger& logger, EventBase& event_base,
                    const LightStepTracerOptions& tracer_options,
                    const StreamRecorderOptions& recorder_options,
                    ChunkCircularBuffer& span_buffer);

  Logger& logger() const noexcept { return logger_; }

  EventBase& event_base() const noexcept { return event_base_; }

  const StreamRecorderOptions& recorder_options() const noexcept {
    return recorder_options_;
  }

  SatelliteEndpointManager& endpoint_manager() noexcept {
    return endpoint_manager_;
  }

  void Flush() noexcept;

  void OnConnectionWritable(SatelliteConnection* connection) noexcept;

 private:
  Logger& logger_;
  EventBase& event_base_;
  const StreamRecorderOptions& recorder_options_;
  SatelliteEndpointManager endpoint_manager_;
  ChunkCircularBuffer& span_buffer_;
  std::vector<std::unique_ptr<SatelliteConnection>> connections_;
  std::vector<SatelliteConnection*> writable_connections_;

  void OnEndpointManagerReady() noexcept;
};
}  // namespace lightstep

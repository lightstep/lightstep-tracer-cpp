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

  /**
   * @return the associated Logger.
   */
  Logger& logger() const noexcept { return logger_; }

  /**
   * @return the associated EventBase.
   */
  EventBase& event_base() const noexcept { return event_base_; }

  /**
   * @return the associated StreamRecorderOptions.
   */
  const StreamRecorderOptions& recorder_options() const noexcept {
    return recorder_options_;
  }

  /**
   * @return an SatelliteEndpointManager for load balancing.
   */
  SatelliteEndpointManager& endpoint_manager() noexcept {
    return endpoint_manager_;
  }

  /**
   * Flush data to satellites if connections are available.
   */
  void Flush() noexcept;

  /**
   * Callback to indicate that a satellite connection is ready for writing.
   * @param SatelliteConnection the writable satellite connection.
   */
  void OnConnectionWritable(SatelliteConnection* connection) noexcept;

  /**
   * Callback to indicate that a satellite connection is no longer writable.
   * @param SatelliteConnection the non-writable satellite connection.
   */
  void OnConnectionNonwritable(SatelliteConnection* connection) noexcept;

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

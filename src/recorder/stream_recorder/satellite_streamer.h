#pragma once

#include <memory>
#include <vector>

#include "common/noncopyable.h"
#include "common/random_traverser.h"
#include "recorder/metrics_tracker.h"
#include "recorder/stream_recorder/satellite_connection.h"
#include "recorder/stream_recorder/satellite_endpoint_manager.h"
#include "recorder/stream_recorder/span_stream.h"

namespace lightstep {
/**
 * Manages the stream of tracing data sent to a pool of satellite connections.
 */
class SatelliteStreamer : private Noncopyable {
 public:
  SatelliteStreamer(Logger& logger, EventBase& event_base,
                    const LightStepTracerOptions& tracer_options,
                    const StreamRecorderOptions& recorder_options,
                    MetricsTracker& metrics,
                    CircularBuffer<ChainedStream>& span_buffer);

  /**
   * @return the associated Logger.
   */
  Logger& logger() const noexcept { return logger_; }

  /**
   * @return the associated EventBase.
   */
  EventBase& event_base() const noexcept { return event_base_; }

  /**
   * @return the associated LightStepTracerOptions
   */
  const LightStepTracerOptions& tracer_options() const noexcept {
    return tracer_options_;
  }

  /**
   * @return the associated StreamRecorderOptions.
   */
  const StreamRecorderOptions& recorder_options() const noexcept {
    return recorder_options_;
  }

  /**
   * @return the SpanStream formed from the StreamRecorder's message buffer.
   */
  SpanStream& span_stream() noexcept { return span_stream_; }

  /**
   * @return the fragment with the common serialization data sent in
   * ReportRequests.
   */
  std::pair<void*, int> header_common_fragment() const noexcept {
    return {
        static_cast<void*>(const_cast<char*>(header_common_fragment_.data())),
        static_cast<int>(header_common_fragment_.size())};
  }

  /**
   * @return an SatelliteEndpointManager for load balancing.
   */
  SatelliteEndpointManager& endpoint_manager() noexcept {
    return endpoint_manager_;
  }

  /**
   * @return true any satellite connections have open sockets.
   */
  bool is_active() const noexcept;

  /**
   * Flush data to satellites if connections are available.
   */
  void Flush() noexcept;

  /**
   * Cleanly shut down satellite connections.
   */
  void InitiateShutdown() noexcept;

 private:
  Logger& logger_;
  EventBase& event_base_;
  const LightStepTracerOptions& tracer_options_;
  const StreamRecorderOptions& recorder_options_;
  std::string header_common_fragment_;
  SatelliteEndpointManager endpoint_manager_;
  CircularBuffer<ChainedStream>& span_buffer_;
  SpanStream span_stream_;
  std::vector<std::unique_ptr<SatelliteConnection>> connections_;
  RandomTraverser connection_traverser_;

  void OnEndpointManagerReady() noexcept;
};
}  // namespace lightstep

#pragma once

#include <memory>
#include <vector>

#include "common/chunk_circular_buffer.h"
#include "common/noncopyable.h"
#include "recorder/stream_recorder/satellite_connection.h"
#include "recorder/stream_recorder/satellite_endpoint_manager.h"
#include "recorder/stream_recorder/span_stream.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
class SatelliteStreamer : private Noncopyable {
 public:
  SatelliteStreamer(Logger& logger, EventBase& event_base,
                    const LightStepTracerOptions& tracer_options,
                    const StreamRecorderOptions& recorder_options,
                    StreamRecorderMetrics& metrics,
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
   * @return the metrics associated with StreamRecorder.
   */
  StreamRecorderMetrics& metrics() const noexcept { return metrics_; }

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
   * Flush data to satellites if connections are available.
   */
  void Flush() noexcept;

 private:
  Logger& logger_;
  EventBase& event_base_;
  const LightStepTracerOptions& tracer_options_;
  const StreamRecorderOptions& recorder_options_;
  std::string header_common_fragment_;
  SatelliteEndpointManager endpoint_manager_;
  StreamRecorderMetrics& metrics_;
  ChunkCircularBuffer& span_buffer_;
  SpanStream span_stream_;
  std::vector<std::unique_ptr<SatelliteConnection>> connections_;

  void OnEndpointManagerReady() noexcept;
};
}  // namespace lightstep

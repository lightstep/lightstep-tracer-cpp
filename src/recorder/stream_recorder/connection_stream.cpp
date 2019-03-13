#include "recorder/stream_recorder/connection_stream.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ConnectionStream::ConnectionStream(Fragment header_common_fragment,
                                   StreamRecorderMetrics& metrics,
                                   SpanStream& span_stream)
    : header_common_fragment_{header_common_fragment},
      metrics_{metrics},
      span_stream_{span_stream} {}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void ConnectionStream::Reset() { InitializeStream(); }

//--------------------------------------------------------------------------------------------------
// Shutdown
//--------------------------------------------------------------------------------------------------
void ConnectionStream::Shutdown() noexcept { shutdown_called_ = true; }

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
bool ConnectionStream::Flush(Writer writer) {
  (void)writer;
  return true;
}

//--------------------------------------------------------------------------------------------------
// empty
//--------------------------------------------------------------------------------------------------
bool ConnectionStream::empty() const noexcept { return true; }

//--------------------------------------------------------------------------------------------------
// InitializeStream
//--------------------------------------------------------------------------------------------------
void ConnectionStream::InitializeStream() { shutdown_called_ = false; }
}  // namespace lightstep

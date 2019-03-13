#include "recorder/stream_recorder/connection_stream.h"

#include <cassert>
#include <cstdio>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// TerminalFragment
//--------------------------------------------------------------------------------------------------
static const Fragment TerminalFragment{
    static_cast<void*>(const_cast<char*>("0\r\n\r\n")), 5};

//--------------------------------------------------------------------------------------------------
// EndOfLineFragment
//--------------------------------------------------------------------------------------------------
static const Fragment EndOfLineFragment{
    static_cast<void*>(const_cast<char*>("\r\n")), 2};

//--------------------------------------------------------------------------------------------------
// WriteChunkHeader
//--------------------------------------------------------------------------------------------------
static Fragment WriteChunkHeader(char* buffer, size_t buffer_size,
                                 int chunk_size) {
  auto num_chunk_header_chars =
      std::snprintf(buffer, buffer_size, "%llX\r\n",
                    static_cast<unsigned long long>(chunk_size));
  assert(num_chunk_header_chars > 0);
  return {static_cast<void*>(buffer), static_cast<int>(num_chunk_header_chars)};
}

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
void ConnectionStream::Reset() {
  if (!header_stream_.empty()) {
    // We weren't able to upload the metrics so add the counters back
    metrics_.num_dropped_spans += embedded_metrics_message_.num_dropped_spans();
  }
  InitializeStream();
}

//--------------------------------------------------------------------------------------------------
// Shutdown
//--------------------------------------------------------------------------------------------------
void ConnectionStream::Shutdown() noexcept { shutting_down_ = true; }

//--------------------------------------------------------------------------------------------------
// Flush
//--------------------------------------------------------------------------------------------------
bool ConnectionStream::Flush(Writer writer) {
  (void)writer;
  return true;
}

//--------------------------------------------------------------------------------------------------
// InitializeStream
//--------------------------------------------------------------------------------------------------
void ConnectionStream::InitializeStream() {
  shutting_down_ = false;
  terminal_stream_ = {TerminalFragment};

  embedded_metrics_message_.set_num_dropped_spans(
      metrics_.num_dropped_spans.exchange(0));
  auto metrics_fragment = embedded_metrics_message_.MakeFragment();

  auto header_chunk_size =
      header_common_fragment_.second + metrics_fragment.second;
  header_stream_ = {
      WriteChunkHeader(chunk_header_buffer_.data(), chunk_header_buffer_.size(),
                       header_chunk_size),
      header_common_fragment_, metrics_fragment, EndOfLineFragment};
}
}  // namespace lightstep

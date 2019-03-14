#include "recorder/stream_recorder/connection_stream.h"

#include <cassert>
#include <cstdio>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// TerminalFragment
//--------------------------------------------------------------------------------------------------
static const Fragment TerminalFragment = MakeFragment("0\r\n\r\n");

//--------------------------------------------------------------------------------------------------
// EndOfLineFragment
//--------------------------------------------------------------------------------------------------
static const Fragment EndOfLineFragment = MakeFragment("\r\n");

//--------------------------------------------------------------------------------------------------
// HttpRequestCommonFragment
//--------------------------------------------------------------------------------------------------
static const Fragment HttpRequestCommonFragment = MakeFragment(
    "POST /reports HTTP/1.1\r\n"
    "Transfer-Encoding:chunked\r\n");

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
ConnectionStream::ConnectionStream(Fragment host_header_fragment,
                                   Fragment header_common_fragment,
                                   StreamRecorderMetrics& metrics,
                                   SpanStream& span_stream)
    : host_header_fragment_{host_header_fragment},
      header_common_fragment_{header_common_fragment},
      metrics_{metrics},
      span_stream_{span_stream} {
  InitializeStream();
}

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
  if (shutting_down_) {
    return writer({&header_stream_, &terminal_stream_});
  }
  return writer({&header_stream_, &span_stream_});
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
      HttpRequestCommonFragment,
      host_header_fragment_,
      EndOfLineFragment,
      WriteChunkHeader(chunk_header_buffer_.data(), chunk_header_buffer_.size(),
                       header_chunk_size),
      header_common_fragment_,
      metrics_fragment,
      EndOfLineFragment};
}
}  // namespace lightstep

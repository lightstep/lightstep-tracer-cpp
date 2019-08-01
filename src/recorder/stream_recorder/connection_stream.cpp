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
    "POST /api/v2/reports HTTP/1.1\r\n"
    "Connection:close\r\n"
    "Content-Type:application/octet-stream\r\n"
    "Transfer-Encoding:chunked\r\n");

//--------------------------------------------------------------------------------------------------
// WriteChunkHeader
//--------------------------------------------------------------------------------------------------
static Fragment WriteChunkHeader(char* buffer, size_t buffer_size,
                                 int chunk_size) noexcept {
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
                                   SpanStream& span_stream)
    : host_header_fragment_{std::move(host_header_fragment)},
      header_common_fragment_{std::move(header_common_fragment)},
      span_stream_{span_stream} {
  InitializeStream();
}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void ConnectionStream::Reset() {
  if (!header_stream_.empty()) {
    // We weren't able to upload the metrics so add the counters back
    span_stream_.metrics().UnconsumeDroppedSpans(
        embedded_metrics_message_.num_dropped_spans());
  }
  if (span_remnant_ != nullptr && !span_remnant_->empty()) {
    span_stream_.metrics().OnSpansDropped(1);
  }
  span_remnant_.reset();
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
    return FlushShutdown(writer);
  }
  span_stream_.Allot();
  if (span_remnant_ == nullptr) {
    auto result = writer({&header_stream_, &span_stream_});
    span_remnant_ = span_stream_.ConsumeRemnant();
    return result;
  }
  auto result = writer({&header_stream_, span_remnant_.get(), &span_stream_});
  if (span_remnant_->empty()) {
    span_stream_.metrics().OnSpansSent(1);
    span_remnant_ = span_stream_.ConsumeRemnant();
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// InitializeStream
//--------------------------------------------------------------------------------------------------
void ConnectionStream::InitializeStream() {
  shutting_down_ = false;
  terminal_stream_ = {TerminalFragment};

  embedded_metrics_message_.set_num_dropped_spans(
      span_stream_.metrics().ConsumeDroppedSpans());
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

//--------------------------------------------------------------------------------------------------
// first_chunk_position
//--------------------------------------------------------------------------------------------------
int ConnectionStream::first_chunk_position() const noexcept {
  return HttpRequestCommonFragment.second + host_header_fragment_.second +
         EndOfLineFragment.second;
}

//--------------------------------------------------------------------------------------------------
// FlushShutdown
//--------------------------------------------------------------------------------------------------
bool ConnectionStream::FlushShutdown(Writer writer) {
  if (span_remnant_ == nullptr) {
    return writer({&header_stream_, &terminal_stream_});
  }
  auto result =
      writer({&header_stream_, span_remnant_.get(), &terminal_stream_});
  if (span_remnant_->empty()) {
    span_stream_.metrics().OnSpansSent(1);
    span_remnant_.reset();
  }
  return result;
}
}  // namespace lightstep

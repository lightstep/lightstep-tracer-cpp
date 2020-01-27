#pragma once

#include <array>
#include <initializer_list>
#include <tuple>

#include "common/fragment_array_input_stream.h"
#include "common/fragment_input_stream.h"
#include "common/function_ref.h"
#include "common/hex_conversion.h"
#include "common/utility.h"
#include "recorder/serialization/embedded_metrics_message.h"
#include "recorder/stream_recorder/span_stream.h"

namespace lightstep {
/**
 * Manages the data to send over a streaming satellite connection.
 */
class ConnectionStream {
  // Account for the size encoded in hex plus the following \r\n sequence.
  static const size_t MaxChunkHeaderSize = Num64BitHexDigits + 2;

 public:
  using Writer = FunctionRef<bool(
      std::initializer_list<FragmentInputStream*> fragment_input_streams)>;

  ConnectionStream(Fragment host_header_fragment,
                   Fragment header_common_fragment, SpanStream& span_stream);

  /**
   * Reset so as to begin a new streaming session.
   */
  void Reset();

  /**
   * Set the current streaming session to end.
   */
  void Shutdown() noexcept;

  /**
   * @return true if the stream is in the process of shutting down.
   */
  bool shutting_down() const noexcept { return shutting_down_; }

  /**
   * Flushes data from the stream.
   * @param writer the writer to flush data to.
   * @return true if all the data in the stream was flushed; false, otherwise.
   */
  bool Flush(Writer writer);

  /**
   * @return true if the streaming session is complete.
   */
  bool completed() const noexcept { return terminal_stream_.empty(); }

  /**
   * @return the number of bytes in the stream until the first chunk.
   */
  int first_chunk_position() const noexcept;

 private:
  Fragment host_header_fragment_;
  Fragment header_common_fragment_;
  SpanStream& span_stream_;
  std::array<char, MaxChunkHeaderSize + 1>
      chunk_header_buffer_;  // Note: Use 1 greater than the actual size so that
                             // we can utilize the snprintf library functions
                             // that add a null terminator.
  EmbeddedMetricsMessage embedded_metrics_message_;

  FragmentArrayInputStream header_stream_;
  FragmentArrayInputStream terminal_stream_;

  std::unique_ptr<ChainedStream> span_remnant_;

  bool shutting_down_;

  void InitializeStream();

  bool FlushShutdown(Writer writer);
};
}  // namespace lightstep

#pragma once

#include <array>
#include <initializer_list>
#include <tuple>

#include "common/function_ref.h"
#include "common/utility.h"
#include "network/fixed_fragment_input_stream.h"
#include "network/fragment_set.h"
#include "recorder/stream_recorder/embedded_metrics_message.h"
#include "recorder/stream_recorder/span_stream.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
class ConnectionStream {
  // Account for the size encoded in hex plus the following \r\n sequence.
  static const size_t MaxChunkHeaderSize = Num64BitHexDigits + 2;

 public:
  using Writer = FunctionRef<std::tuple<int, int, int>(
      std::initializer_list<const FragmentSet*> fragment_sets)>;

  ConnectionStream(Fragment header_common_fragment,
                   StreamRecorderMetrics& metrics, SpanStream& span_stream);

  void Reset();

  void Shutdown() noexcept;

  bool Flush(Writer writer);

 private:
  Fragment header_common_fragment_;
  StreamRecorderMetrics& metrics_;
  SpanStream& span_stream_;
  std::array<char, MaxChunkHeaderSize + 1>
      chunk_header_buffer_;  // Note: Use 1 greater than the actual size so that
                             // we can utilize the snprintf library functions
                             // that add a null terminator.
  EmbeddedMetricsMessage embedded_metrics_message_;

  FixedFragmentInputStream<4> header_stream_;
  FixedFragmentInputStream<1> terminal_stream_;

  bool shutting_down_;

  void InitializeStream();
};
}  // namespace lightstep

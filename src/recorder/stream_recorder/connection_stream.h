#pragma once

#include <initializer_list>
#include <tuple>

#include "common/function_ref.h"
#include "network/fragment_set.h"
#include "recorder/stream_recorder/span_stream.h"
#include "recorder/stream_recorder/stream_recorder_metrics.h"

namespace lightstep {
class ConnectionStream {
 public:
  using Writer = FunctionRef<std::tuple<int, int, int>(
      std::initializer_list<const FragmentSet*> fragment_sets)>;

  ConnectionStream(Fragment header_common_fragment,
                   StreamRecorderMetrics& metrics, SpanStream& span_stream);

  void Reset();

  void Shutdown() noexcept;

  bool Flush(Writer writer);

  bool empty() const noexcept;

 private:
  Fragment header_common_fragment_;
  StreamRecorderMetrics& metrics_;
  SpanStream& span_stream_;

  bool shutdown_called_;

  void InitializeStream();
};
}  // namespace lightstep

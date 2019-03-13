#include "recorder/stream_recorder/span_stream.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream::SpanStream(ChunkCircularBuffer& span_buffer) noexcept
    : span_buffer_{span_buffer} {}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int SpanStream::num_fragments() const noexcept { return 0; }

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SpanStream::ForEachFragment(
    FunctionRef<FragmentSet::Callback> /*callback*/) const noexcept {
  return true;
}
}  // namespace lightstep

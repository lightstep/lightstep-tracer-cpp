#include "recorder/stream_recorder/fragment_span_input_stream.h"

#include <cassert>

#include "recorder/stream_recorder/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
FragmentSpanInputStream::FragmentSpanInputStream() {
  stream_.Reserve(2);
}

//--------------------------------------------------------------------------------------------------
// Set
//--------------------------------------------------------------------------------------------------
void FragmentSpanInputStream::Set(const CircularBufferConstPlacement& chunk,
                                  const char* position) {
  stream_.Clear();
  chunk_start_ = chunk.data1;
  if (Contains(chunk.data1, chunk.size1, position)) {
    stream_.Add(Fragment{
        static_cast<void*>(const_cast<char*>(position)),
        static_cast<int>(std::distance(position, chunk.data1 + chunk.size1))});
    if (chunk.size2 > 0) {
      stream_.Add(Fragment{static_cast<void*>(const_cast<char*>(chunk.data2)),
                           static_cast<int>(chunk.size2)});
    }
    return;
  }
  assert(Contains(chunk.data2, chunk.size2, position));
  stream_.Add(Fragment{
      static_cast<void*>(const_cast<char*>(position)),
      static_cast<int>(std::distance(position, chunk.data2 + chunk.size2))});
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void FragmentSpanInputStream::Clear() noexcept {
  chunk_start_ = nullptr;
  return stream_.Clear();
}
} // namespace lightstep

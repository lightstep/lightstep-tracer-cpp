#include "recorder/stream_recorder/span_stream.h"

#include <cassert>
#include <iterator>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Contains
//--------------------------------------------------------------------------------------------------
static bool Contains(const char* data, size_t size, const char* ptr) {
  if (data == nullptr) {
    return false;
  }
  if (ptr == nullptr) {
    return false;
  }
  auto delta = std::distance(data, ptr);
  if (delta < 0) {
    return false;
  }
  return static_cast<size_t>(delta) < size;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream::SpanStream(ChunkCircularBuffer& span_buffer) noexcept
    : span_buffer_{span_buffer} {}

//--------------------------------------------------------------------------------------------------
// Allot
//--------------------------------------------------------------------------------------------------
void SpanStream::Allot() noexcept {
  span_buffer_.Allot();
  allotment_ = span_buffer_.allotment();
  if (stream_position_ == nullptr) {
    stream_position_ = allotment_.data1;
  }
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int SpanStream::num_fragments() const noexcept {
  if (Contains(allotment_.data1, allotment_.size1, stream_position_)) {
    return 1 + static_cast<int>(allotment_.size2 > 0);
  }
  return static_cast<int>(
      Contains(allotment_.data2, allotment_.size2, stream_position_));
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SpanStream::ForEachFragment(Callback callback) const noexcept {
  if (Contains(allotment_.data1, allotment_.size1, stream_position_)) {
    auto result =
        callback(static_cast<void*>(const_cast<char*>(stream_position_)),
                 static_cast<int>(allotment_.size1) -
                     static_cast<int>(
                         std::distance(allotment_.data1, stream_position_)));
    if (!result) {
      return false;
    }
    if (allotment_.size2 > 0) {
      return callback(static_cast<void*>(const_cast<char*>(allotment_.data2)),
                      static_cast<int>(allotment_.size2));
    }
    return true;
  }
  if (Contains(allotment_.data2, allotment_.size2, stream_position_)) {
    return callback(static_cast<void*>(const_cast<char*>(stream_position_)),
                    static_cast<int>(allotment_.size2) -
                        static_cast<int>(
                            std::distance(allotment_.data2, stream_position_)));
  }
  return true;
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void SpanStream::Clear() noexcept {}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SpanStream::Seek(int /*fragment_index*/, int /*position*/) noexcept {}
}  // namespace lightstep

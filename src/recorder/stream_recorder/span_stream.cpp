#include "recorder/stream_recorder/span_stream.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "recorder/stream_recorder/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream::SpanStream(ChunkCircularBuffer& span_buffer)
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
// RemoveRemnant
//--------------------------------------------------------------------------------------------------
void SpanStream::RemoveRemnant(const char* remnant) noexcept {
  auto last = std::remove(remnants_.begin(), remnants_.end(), remnant);
  assert(last != remnants_.end());
  remnants_.erase(last, remnants_.end());
}

//--------------------------------------------------------------------------------------------------
// PopSpanRemnant
//--------------------------------------------------------------------------------------------------
void SpanStream::PopSpanRemnant(FragmentSpanInputStream& remnant) noexcept {
  std::swap(remnant, span_remnant_);
  span_remnant_.Clear();
}

//--------------------------------------------------------------------------------------------------
// Consume
//--------------------------------------------------------------------------------------------------
void SpanStream::Consume() noexcept {
      
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
void SpanStream::Clear() noexcept { SetPositionAfter(allotment_); }

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SpanStream::Seek(int fragment_index, int position) noexcept {
  const char* last;
  if (fragment_index == 0) {
    last = stream_position_ + position;
  } else {
    assert(fragment_index == 1);
    assert(Contains(allotment_.data2, allotment_.size2, stream_position_));
    last = allotment_.data2 + position;
    assert(Contains(allotment_.data2, allotment_.size2, last));
  }
  auto chunk = span_buffer_.FindChunk(stream_position_, last);

  span_remnant_.Clear();

  // If last is at the start of the next chunk, we don't end up with any partial
  // span that we need to track.
  if (chunk.data1 == last) {
    stream_position_ = last;
    return;
  }

  remnants_.emplace_back(chunk.data1);
  span_remnant_.Set(chunk, last);
  SetPositionAfter(chunk);
}

//--------------------------------------------------------------------------------------------------
// SetPositionAfter
//--------------------------------------------------------------------------------------------------
void SpanStream::SetPositionAfter(
    const CircularBufferConstPlacement& placement) noexcept {
  const char* last;
  if (placement.size2 > 0) {
    last = placement.data2 + placement.size2;
  } else {
    last = placement.data1 + placement.size1;
  }

  // Handle the case when last would wrap the end of the circular buffer.
  //
  // Note that max_size is one less than the actual amount of memory allocated
  // for the circular buffer.
  if (last == span_buffer_.data() + span_buffer_.max_size() + 1) {
    last = span_buffer_.data();
  }
}
}  // namespace lightstep

#include "recorder/stream_recorder/span_stream.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "recorder/stream_recorder/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream::SpanStream(ChunkCircularBuffer& span_buffer,
                       StreamRecorderMetrics& metrics, int max_connections)
    : span_buffer_{span_buffer}, metrics_{metrics} {
  span_remnants_.reserve(static_cast<size_t>(max_connections));
}

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
// RemoveSpanRemnant
//--------------------------------------------------------------------------------------------------
void SpanStream::RemoveSpanRemnant(const char* remnant) noexcept {
  auto last =
      std::remove(span_remnants_.begin(), span_remnants_.end(), remnant);
  assert(last != span_remnants_.end());
  span_remnants_.erase(last, span_remnants_.end());
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
  auto num_bytes = span_buffer_.buffer().ComputePosition(stream_position_);
  for (auto remnant : span_remnants_) {
    num_bytes =
        std::min(span_buffer_.buffer().ComputePosition(remnant), num_bytes);
  }
  span_buffer_.Consume(num_bytes);
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SpanStream::ForEachFragment(Callback callback) const noexcept {
  if (Contains(allotment_.data1, allotment_.size1, stream_position_)) {
    auto result =
        callback(static_cast<void*>(const_cast<char*>(stream_position_)),
                 static_cast<int>(std::distance(
                     stream_position_, allotment_.data1 + allotment_.size1)));
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
    return callback(
        static_cast<void*>(const_cast<char*>(stream_position_)),
        static_cast<int>(std::distance(stream_position_,
                                       allotment_.data2 + allotment_.size2)));
  }
  return true;
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void SpanStream::Clear() noexcept {
  SetPositionAfter(allotment_);
  span_remnant_.Clear();
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SpanStream::Seek(int fragment_index, int position) noexcept {
  const char* last;
  if (fragment_index == 0) {
    last = stream_position_ + position;
  } else {
    assert(fragment_index == 1);
    assert(Contains(allotment_.data1, allotment_.size1, stream_position_));
    last = allotment_.data2 + position;
    assert(Contains(allotment_.data2, allotment_.size2, last));
  }
  CircularBufferConstPlacement chunk;
  int num_preceding_chunks;
  std::tie(chunk, num_preceding_chunks) =
      span_buffer_.FindChunk(stream_position_, last);
  (void)num_preceding_chunks;

  span_remnant_.Clear();

  // If last is at the start of the next chunk, we don't end up with any partial
  // span that we need to track.
  if (chunk.data1 == last) {
    stream_position_ = last;
    return;
  }

  span_remnants_.emplace_back(chunk.data1);
  span_remnant_.Set(chunk, last);
  SetPositionAfter(chunk);
}

//--------------------------------------------------------------------------------------------------
// SetPositionAfter
//--------------------------------------------------------------------------------------------------
void SpanStream::SetPositionAfter(
    const CircularBufferConstPlacement& placement) noexcept {
  if (placement.size2 > 0) {
    stream_position_ = placement.data2 + placement.size2;
  } else {
    stream_position_ = placement.data1 + placement.size1;
  }

  // Handle the case when stream_position_ would wrap the end of the circular
  // buffer.
  //
  // Note that max_size is one less than the actual amount of memory allocated
  // for the circular buffer.
  if (stream_position_ ==
      span_buffer_.buffer().data() + span_buffer_.buffer().max_size() + 1) {
    stream_position_ = span_buffer_.buffer().data();
  }
}
}  // namespace lightstep

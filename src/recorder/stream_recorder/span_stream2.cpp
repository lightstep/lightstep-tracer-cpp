#include "recorder/stream_recorder/span_stream2.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream2::SpanStream2(CircularBuffer2<SerializationChain>& span_buffer,
                         StreamRecorderMetrics& metrics) noexcept
    : span_buffer_{span_buffer}, metrics_{metrics} {}

//--------------------------------------------------------------------------------------------------
// Allot
//--------------------------------------------------------------------------------------------------
void SpanStream2::Allot() noexcept {
  allotment_ = span_buffer_.Peek();
}

//--------------------------------------------------------------------------------------------------
// ConsumeRemnant
//--------------------------------------------------------------------------------------------------
std::unique_ptr<SerializationChain> SpanStream2::ConsumeRemnant() noexcept {
  return std::unique_ptr<SerializationChain>{remnant_.release()};
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int SpanStream2::num_fragments() const noexcept {
  int result = 0;
  allotment_.ForEach([&result](
      const AtomicUniquePtr<SerializationChain>& span) noexcept {
    result += span->num_fragments();
    return true;
  });
  return result;
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SpanStream2::ForEachFragment(Callback callback) const noexcept {
  return allotment_.ForEach(
      [callback](const AtomicUniquePtr<SerializationChain>& span) {
        if (!span->ForEachFragment(callback)) {
          return false;
        }
        return true;
      });
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void SpanStream2::Clear() noexcept {
  remnant_.reset();
  span_buffer_.Consume(allotment_.size());  
  allotment_ = CircularBufferRange<const AtomicUniquePtr<SerializationChain>>{};
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SpanStream2::Seek(int fragment_index, int position) noexcept {
  remnant_.reset();
  int span_count = 0;
  allotment_.ForEach([ fragment_index, &span_count, position ](
      const AtomicUniquePtr<SerializationChain>& span) mutable noexcept {
    auto num_fragments = span->num_fragments();
    if (num_fragments <= fragment_index) {
      fragment_index -= num_fragments;
      ++span_count;
      return true;
    }
    // Check to see if we seek onto a span boundary -- in which case, we don't
    // consume the the next span.
    if (fragment_index == 0 && position == 0) {
      return false;
    }
    ++span_count;
    return false;
  });
  span_buffer_.Consume(
      span_count, [ this, fragment_index, position ](
                      CircularBufferRange<AtomicUniquePtr<SerializationChain>>
                          range) mutable noexcept {
        range.ForEach([&](AtomicUniquePtr<SerializationChain> & span) noexcept {
          auto num_fragments = span->num_fragments();
          if (num_fragments <= fragment_index) {
            fragment_index -= num_fragments;
            span.Reset();
            return true;
          }
          span->Seek(fragment_index, position);
          span.Swap(remnant_);
          return true;
        });
      });
  allotment_ = CircularBufferRange<const AtomicUniquePtr<SerializationChain>>{};
}
}  // namespace lightstep

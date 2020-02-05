#include "recorder/stream_recorder/span_stream.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
SpanStream::SpanStream(CircularBuffer<ChainedStream>& span_buffer,
                       MetricsTracker& metrics) noexcept
    : span_buffer_{span_buffer}, metrics_{metrics} {}

//--------------------------------------------------------------------------------------------------
// Allot
//--------------------------------------------------------------------------------------------------
void SpanStream::Allot() noexcept { allotment_ = span_buffer_.Peek(); }

//--------------------------------------------------------------------------------------------------
// ConsumeRemnant
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ChainedStream> SpanStream::ConsumeRemnant() noexcept {
  return std::unique_ptr<ChainedStream>{remnant_.release()};
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int SpanStream::num_fragments() const noexcept {
  int result = 0;
  allotment_.ForEach([&result](
      const AtomicUniquePtr<ChainedStream>& span) noexcept {
    result += span->num_fragments();
    return true;
  });
  return result;
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool SpanStream::ForEachFragment(Callback callback) const noexcept {
  return allotment_.ForEach(
      [callback](const AtomicUniquePtr<ChainedStream>& span) {
        return span->ForEachFragment(callback);
      });
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void SpanStream::Clear() noexcept {
  remnant_.reset();
  metrics_.OnSpansSent(allotment_.size());
  span_buffer_.Consume(allotment_.size());
  allotment_ = CircularBufferRange<const AtomicUniquePtr<ChainedStream>>{};
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void SpanStream::Seek(int fragment_index, int position) noexcept {
  remnant_.reset();
  int full_span_count = 0;
  int span_count = 0;
  allotment_.ForEach([&, fragment_index ](
      const AtomicUniquePtr<ChainedStream>& span) mutable noexcept {
    auto num_fragments = span->num_fragments();
    if (num_fragments <= fragment_index) {
      fragment_index -= num_fragments;
      ++span_count;
      return true;
    }
    full_span_count = span_count;
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
                      CircularBufferRange<AtomicUniquePtr<ChainedStream>>
                          range) mutable noexcept {
        range.ForEach([&](AtomicUniquePtr<ChainedStream> & span) noexcept {
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
  metrics_.OnSpansSent(full_span_count);
  allotment_ = CircularBufferRange<const AtomicUniquePtr<ChainedStream>>{};
}
}  // namespace lightstep

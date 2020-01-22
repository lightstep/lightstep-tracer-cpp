#include "common/composable_fragment_input_stream.h"

#include <cassert>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Append
//--------------------------------------------------------------------------------------------------
void ComposableFragmentInputStream::Append(
    std::unique_ptr<ComposableFragmentInputStream>&& stream) noexcept {
  if (next_ == nullptr) {
    next_ = std::move(stream);
    last_ = next_.get();
    return;
  }
  assert(last_ != nullptr);
  auto prev_last = last_;
  last_ = stream.get();
  prev_last->Append(std::move(stream));
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int ComposableFragmentInputStream::num_fragments() const noexcept {
  auto stream = this;
  int result = 0;
  while (stream != nullptr) {
    result += stream->segment_num_fragments();
    stream = stream->next_.get();
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool ComposableFragmentInputStream::ForEachFragment(Callback callback) const
    noexcept {
  auto stream = this;
  while (stream != nullptr) {
    if (!stream->SegmentForEachFragment(callback)) {
      return false;
    }
    stream = stream->next_.get();
  }
  return true;
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void ComposableFragmentInputStream::Clear() noexcept {
  auto stream = this;
  while (stream != nullptr) {
    stream->SegmentClear();
    stream = stream->next_.get();
  }
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void ComposableFragmentInputStream::Seek(int fragment_index,
                                         int position) noexcept {
  auto stream = this;
  while (stream != nullptr) {
    auto segment_num_fragments = stream->segment_num_fragments();
    if (fragment_index < segment_num_fragments) {
      return stream->SegmentSeek(fragment_index, position);
    }
    fragment_index -= segment_num_fragments;
    stream->SegmentClear();
    stream = stream->next_.get();
  }
  assert(fragment_index == 0 && position == 0);
}
}  // namespace lightstep

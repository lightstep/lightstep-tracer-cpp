#pragma once

#include <memory>

#include "common/composable_fragment_input_stream.h"

namespace lightstep {
/**
 * Wraps a FragmentInputStream to make it composable for testing.
 */
class ComposableFragmentInputStreamWrapper final
    : public ComposableFragmentInputStream {
 public:
  explicit ComposableFragmentInputStreamWrapper(
      std::unique_ptr<FragmentInputStream>&& stream) noexcept
      : stream_{std::move(stream)} {}

  // ComposableFragmentInputStream
  int segment_num_fragments() const noexcept override {
    return stream_->num_fragments();
  }

  bool SegmentForEachFragment(Callback callback) const noexcept override {
    return stream_->ForEachFragment(callback);
  }

  void SegmentClear() noexcept override { return stream_->Clear(); }

  void SegmentSeek(int fragment_index, int position) noexcept override {
    return stream_->Seek(fragment_index, position);
  }

 private:
  std::unique_ptr<FragmentInputStream> stream_;
};
}  // namespace lightstep

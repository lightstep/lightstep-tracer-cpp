#pragma once

#include "common/circular_buffer.h"
#include "network/fragment_array_input_stream.h"

namespace lightstep {
/**
 * Wraps a FragmentArrayInputStream for a span chunk.
 */
class FragmentSpanInputStream final : public FragmentInputStream {
 public:
  FragmentSpanInputStream();

  /**
   * Sets the span for the fragments.
   * @param chunk the placement for the span in the circular buffer.
   * @param position a pointer one past the last byte of the span sent.
   */
  void Set(const CircularBufferConstPlacement& chunk, const char* position);

  /**
   * @return the address of the span's chunk in the circular buffer.
   */
  const char* chunk_start() const noexcept { return chunk_start_; }

  // FragmentInputStream
  int num_fragments() const noexcept override {
    return stream_.num_fragments();
  }

  bool ForEachFragment(Callback callback) const noexcept override {
    return stream_.ForEachFragment(callback);
  }

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override {
    return stream_.Seek(fragment_index, position);
  }

 private:
  const char* chunk_start_{nullptr};
  FragmentArrayInputStream stream_;
};
}  // namespace lightstep

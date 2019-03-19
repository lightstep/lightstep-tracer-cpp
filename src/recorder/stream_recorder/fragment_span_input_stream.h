#pragma once

#include "common/circular_buffer.h"
#include "network/fragment_array_input_stream.h"

namespace lightstep {
class FragmentSpanInputStream final : public FragmentInputStream {
 public:
   FragmentSpanInputStream();

   void Set(const CircularBufferConstPlacement& chunk, const char* position);

   const char* chunk_start() const noexcept { return chunk_start_; }

   // FragmentInputStream
  int num_fragments() const noexcept override { return stream_.num_fragments(); }

  bool ForEachFragment(Callback callback) const noexcept override {
    return stream_.ForEachFragment(callback);
  }

  void Clear() noexcept override;

  void Seek(int fragment_index, int position) noexcept override {
    return stream_.Seek(fragment_index, position);
  }
 private:
   const char* chunk_start_;
   FragmentArrayInputStream stream_;
};
} // namespace lightstep

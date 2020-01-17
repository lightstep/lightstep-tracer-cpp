#pragma once

#include <memory>

#include "common/fragment_input_stream.h"

namespace lightstep {
class ComposableFragmentInputStream : public FragmentInputStream {
 public:
   void Append(std::unique_ptr<ComposableFragmentInputStream>&& stream) noexcept;

   virtual int segment_num_fragments() const noexcept = 0;

   virtual bool SegmentForEachFragment(Callback callback) const noexcept = 0;

   virtual void SegmentClear() noexcept = 0;

   virtual void SegmentSeek(int fragment_index, int position) noexcept = 0;

   // FragmentInputStream
  int num_fragments() const noexcept final;

  bool ForEachFragment(Callback callback) const noexcept final;

  void Clear() noexcept final;

  void Seek(int fragment_index, int position) noexcept final;

 private:
   std::unique_ptr<ComposableFragmentInputStream> next_;
   ComposableFragmentInputStream* last_{nullptr};
};
} // namespace lightstep

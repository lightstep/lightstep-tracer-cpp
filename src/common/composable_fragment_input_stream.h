#pragma once

#include <memory>

#include "common/fragment_input_stream.h"

namespace lightstep {
/**
 * Maintains an intrusive linked list of ComposableFragmentInputStreams.
 */
class ComposableFragmentInputStream : public FragmentInputStream {
 public:
  /**
   * Append a ComposableFragmentInputStream.
   * @param stream the ComposableFragmentInputStream to append
   */
  void Append(std::unique_ptr<ComposableFragmentInputStream>&& stream) noexcept;

  /**
   * The number of fragments in this node of the linked
   * ComposableFragmentInputStreams.
   */
  virtual int segment_num_fragments() const noexcept = 0;

  /**
   * Iterate over the fragments in this node of the linked
   * ComposableFragmentInputStreams.
   * @param callback the callback to call for each fragment
   */
  virtual bool SegmentForEachFragment(Callback callback) const noexcept = 0;

  /**
   * Clear this node of the linked ComposableFragmentInputStreams.
   */
  virtual void SegmentClear() noexcept = 0;

  /**
   * Seek forward in this node of the linked ComposableFragmentInputStreams.
   * @param fragment_index the new fragment index to reposition to relative to
   * the current fragment index.
   * @param position the position within fragment_index to reposition to
   * relative to the current position.
   */
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
}  // namespace lightstep

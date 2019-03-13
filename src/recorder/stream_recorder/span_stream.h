#pragma once

#include "common/chunk_circular_buffer.h"
#include "network/fragment_set.h"

namespace lightstep {
class SpanStream final : public FragmentSet {
 public:
  explicit SpanStream(ChunkCircularBuffer& span_buffer) noexcept;

  // FragmentSet
  int num_fragments() const noexcept override;

  bool ForEachFragment(FunctionRef<FragmentSet::Callback> callback) const
      noexcept override;

 private:
  ChunkCircularBuffer& span_buffer_;
};
}  // namespace lightstep

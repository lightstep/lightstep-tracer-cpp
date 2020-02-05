#pragma once

#include <array>
#include <cassert>
#include <limits>
#include <memory>

#include "common/composable_fragment_input_stream.h"
#include "common/function_ref.h"
#include "common/noncopyable.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace lightstep {
/**
 * Maintains a linked chain of blocks as they are written
 */
class ChainedStream final : public google::protobuf::io::ZeroCopyOutputStream,
                            public ComposableFragmentInputStream,
                            private Noncopyable {
 public:
  static const int BlockSize = 256;

  ChainedStream() noexcept;

  /**
   * Close the stream for output. After calling this, we can no longer write to
   * the stream, but we can interact with it as a FragmentInputStream.
   */
  void CloseOutput() noexcept;

  // ZeroCopyOutputStream
  bool Next(void** data, int* size) override;

  void BackUp(int count) override;

  google::protobuf::int64 ByteCount() const override {
    return static_cast<google::protobuf::int64>(num_bytes_written_);
  }

  // FragmentInputStream
  int segment_num_fragments() const noexcept override;

  bool SegmentForEachFragment(Callback callback) const noexcept override;

  void SegmentClear() noexcept override;

  void SegmentSeek(int relative_fragment_index, int position) noexcept override;

 private:
  struct Block {
    std::unique_ptr<Block> next;
    int size;
    std::array<char, BlockSize> data;
  };

  bool output_closed_{false};

  int num_blocks_{1};
  int num_bytes_written_{0};
  int current_block_position_{0};
  Block* current_block_;

  int fragment_index_{0};
  int fragment_position_{0};

  Block head_;
};
}  // namespace lightstep
